#include "q_shared.h"
#include "mem.h"
#include "cmd.h"

#define	MAX_CMD_BUFFER	16384
#define	MAX_CMD_LINE	1024

typedef struct {
    byte	*data;
    int		maxsize;
    int		cursize;
} cmd_t;

int			cmd_wait;
cmd_t		cmd_text;
byte		cmd_text_buf[MAX_CMD_BUFFER];


/*
=============================================================================

COMMAND EXECUTION

=============================================================================
*/

typedef struct cmd_function_s
{
    struct cmd_function_s	*next;
    char					*name;
    xcommand_t				function;
} cmd_function_t;


static	int			cmd_argc;
static	char		*cmd_argv[MAX_STRING_TOKENS];		// points into cmd_tokenized
static	char		cmd_tokenized[BIG_INFO_STRING + MAX_STRING_TOKENS];	// will have 0 bytes inserted
static	char		cmd_cmd[BIG_INFO_STRING]; // the original command we received (no token processing)

static	cmd_function_t	*cmd_functions;		// possible commands to execute

/*
============
Cmd_Argc
============
*/
int		Cmd_Argc(void) {
    return cmd_argc;
}

/*
============
Cmd_Argv
============
*/
char	*Cmd_Argv(int arg) {
    if ((unsigned)arg >= cmd_argc) {
        return "";
    }
    return cmd_argv[arg];
}

/*
============
Cmd_ArgvBuffer

The interpreted versions use this because
they can't have pointers returned to them
============
*/
void	Cmd_ArgvBuffer(int arg, char *buffer, int bufferLength) {
    Q_strncpyz(buffer, Cmd_Argv(arg), bufferLength);
}


/*
============
Cmd_Args

Returns a single string containing argv(1) to argv(argc()-1)
============
*/
char	*Cmd_Args(void) {
    static	char		cmd_args[MAX_STRING_CHARS];
    int		i;

    cmd_args[0] = 0;
    for (i = 1; i < cmd_argc; i++) {
        strcat(cmd_args, cmd_argv[i]);
        if (i != cmd_argc - 1) {
            strcat(cmd_args, " ");
        }
    }

    return cmd_args;
}

/*
============
Cmd_Args

Returns a single string containing argv(arg) to argv(argc()-1)
============
*/
char *Cmd_ArgsFrom(int arg) {
    static	char		cmd_args[BIG_INFO_STRING];
    int		i;

    cmd_args[0] = 0;
    if (arg < 0)
        arg = 0;
    for (i = arg; i < cmd_argc; i++) {
        strcat(cmd_args, cmd_argv[i]);
        if (i != cmd_argc - 1) {
            strcat(cmd_args, " ");
        }
    }

    return cmd_args;
}

/*
============
Cmd_ArgsBuffer

The interpreted versions use this because
they can't have pointers returned to them
============
*/
void	Cmd_ArgsBuffer(char *buffer, int bufferLength) {
    Q_strncpyz(buffer, Cmd_Args(), bufferLength);
}

/*
============
Cmd_Cmd

Retrieve the unmodified command string
For rcon use when you want to transmit without altering quoting
https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=543
============
*/
char *Cmd_Cmd()
{
    return cmd_cmd;
}

/*
============
Cmd_TokenizeString

Parses the given string into command line tokens.
The text is copied to a seperate buffer and 0 characters
are inserted in the apropriate place, The argv array
will point into this temporary buffer.
============
*/
// NOTE TTimo define that to track tokenization issues
//#define TKN_DBG
void Cmd_TokenizeString(const char *text_in) {
    const char	*text;
    char	*textOut;

#ifdef TKN_DBG
    // FIXME TTimo blunt hook to try to find the tokenization of userinfo
    Com_DPrintf("Cmd_TokenizeString: %s\n", text_in);
#endif

    // clear previous args
    cmd_argc = 0;

    if (!text_in) {
        return;
    }

    Q_strncpyz(cmd_cmd, text_in, sizeof(cmd_cmd));

    text = text_in;
    textOut = cmd_tokenized;

    while (1) {
        if (cmd_argc == MAX_STRING_TOKENS) {
            return;			// this is usually something malicious
        }

        while (1) {
            // skip whitespace
            while (*text && *text <= ' ') {
                text++;
            }
            if (!*text) {
                return;			// all tokens parsed
            }

            // skip // comments
            if (text[0] == '/' && text[1] == '/') {
                return;			// all tokens parsed
            }

            // skip /* */ comments
            if (text[0] == '/' && text[1] == '*') {
                while (*text && (text[0] != '*' || text[1] != '/')) {
                    text++;
                }
                if (!*text) {
                    return;		// all tokens parsed
                }
                text += 2;
            }
            else {
                break;			// we are ready to parse a token
            }
        }

        // handle quoted strings
        // NOTE TTimo this doesn't handle \" escaping
        if (*text == '"') {
            cmd_argv[cmd_argc] = textOut;
            cmd_argc++;
            text++;
            while (*text && *text != '"') {
                *textOut++ = *text++;
            }
            *textOut++ = 0;
            if (!*text) {
                return;		// all tokens parsed
            }
            text++;
            continue;
        }

        // regular token
        cmd_argv[cmd_argc] = textOut;
        cmd_argc++;

        // skip until whitespace, quote, or command
        while (*text > ' ') {
            if (text[0] == '"') {
                break;
            }

            if (text[0] == '/' && text[1] == '/') {
                break;
            }

            // skip /* */ comments
            if (text[0] == '/' && text[1] == '*') {
                break;
            }

            *textOut++ = *text++;
        }

        *textOut++ = 0;

        if (!*text) {
            return;		// all tokens parsed
        }
    }

}

/*
============
Cmd_AddCommand
============
*/
void	Cmd_AddCommand(const char *cmd_name, xcommand_t function) {
    cmd_function_t	*cmd;

    // fail if the command already exists
    for (cmd = cmd_functions; cmd; cmd = cmd->next) {
        if (!strcmp(cmd_name, cmd->name)) {
            // allow completion-only commands to be silently doubled
            if (function != NULL) {
                Com_Printf("Cmd_AddCommand: %s already defined\n", cmd_name);
            }
            return;
        }
    }

    // use a small malloc to avoid zone fragmentation
    cmd = S_Malloc(sizeof(cmd_function_t));
    cmd->name = CopyString(cmd_name);
    cmd->function = function;
    cmd->next = cmd_functions;
    cmd_functions = cmd;
}

/*
============
Cmd_RemoveCommand
============
*/
void	Cmd_RemoveCommand(const char *cmd_name) {
    cmd_function_t	*cmd, **back;

    back = &cmd_functions;
    while (1) {
        cmd = *back;
        if (!cmd) {
            // command wasn't active
            return;
        }
        if (!strcmp(cmd_name, cmd->name)) {
            *back = cmd->next;
            if (cmd->name) {
                Z_Free(cmd->name);
            }
            Z_Free(cmd);
            return;
        }
        back = &cmd->next;
    }
}


/*
============
Cmd_CommandCompletion
============
*/
void	Cmd_CommandCompletion(void(*callback)(const char *s)) {
    cmd_function_t	*cmd;

    for (cmd = cmd_functions; cmd; cmd = cmd->next) {
        callback(cmd->name);
    }
}


/*
============
Cmd_ExecuteString

A complete command line has been parsed, so try to execute it
============
*/
void	Cmd_ExecuteString(const char *text) {
    cmd_function_t	*cmd, **prev;

    // execute the command line
    Cmd_TokenizeString(text);
    if (!Cmd_Argc()) {
        return;		// no tokens
    }

    // check registered command functions	
    for (prev = &cmd_functions; *prev; prev = &cmd->next) {
        cmd = *prev;
        if (!Q_stricmp(cmd_argv[0], cmd->name)) {
            // rearrange the links so that the command will be
            // near the head of the list next time it is used
            *prev = cmd->next;
            cmd->next = cmd_functions;
            cmd_functions = cmd;

            // perform the action
            if (!cmd->function) {
                // let the cgame or game handle it
                break;
            }
            else {
                cmd->function();
            }
            return;
        }
    }

    //// check cvars
    //if (Cvar_Command()) {
    //    return;
    //}

    //// check client game commands
    //if (com_cl_running && com_cl_running->integer && CL_GameCommand()) {
    //    return;
    //}

    //// check server game commands
    //if (com_sv_running && com_sv_running->integer && SV_GameCommand()) {
    //    return;
    //}

    //// check ui commands
    //if (com_cl_running && com_cl_running->integer && UI_GameCommand()) {
    //    return;
    //}

    //// send it as a server command if we are connected
    //// this will usually result in a chat message
    //CL_ForwardCommandToServer(text);
}

/*
============
Cmd_List_f
============
*/
void Cmd_List_f(void)
{
    cmd_function_t	*cmd;
    int				i;
    char			*match;

    if (Cmd_Argc() > 1) {
        match = Cmd_Argv(1);
    }
    else {
        match = NULL;
    }

    i = 0;
    for (cmd = cmd_functions; cmd; cmd = cmd->next) {
        //if (match && !Com_Filter(match, cmd->name, qfalse)) continue;

        Com_Printf("%s\n", cmd->name);
        i++;
    }
    Com_Printf("%i commands\n", i);
}

/*
============
Cmd_Init
============
*/
void Cmd_Init(void) {
    Cmd_AddCommand("cmdlist", Cmd_List_f);
    //Cmd_AddCommand("exec", Cmd_Exec_f);
    //Cmd_AddCommand("vstr", Cmd_Vstr_f);
    //Cmd_AddCommand("echo", Cmd_Echo_f);
    //Cmd_AddCommand("wait", Cmd_Wait_f);
}
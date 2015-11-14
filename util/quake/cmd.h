#ifndef CMD_H_
#define CMD_H_



/*
==============================================================

CMD

Command text buffering and command execution

==============================================================
*/

/*

Any number of commands can be added in a frame, from several different sources.
Most commands come from either keybindings or console line input, but entire text
files can be execed.

*/

void Cbuf_Init(void);
// allocates an initial text buffer that will grow as needed

void Cbuf_AddText(const char *text);
// Adds command text at the end of the buffer, does NOT add a final \n

void Cbuf_ExecuteText(int exec_when, const char *text);
// this can be used in place of either Cbuf_AddText or Cbuf_InsertText

void Cbuf_Execute(void);
// Pulls off \n terminated lines of text from the command buffer and sends
// them through Cmd_ExecuteString.  Stops when the buffer is empty.
// Normally called once per frame, but may be explicitly invoked.
// Do not call inside a command function, or current args will be destroyed.

/*

Command execution takes a null terminated string, breaks it into tokens,
then searches for a command or variable that matches the first token.

*/

typedef void(*xcommand_t) (void);

void	Cmd_Init(void);

void	Cmd_AddCommand(const char *cmd_name, xcommand_t function);
// called by the init functions of other parts of the program to
// register commands and functions to call for them.
// The cmd_name is referenced later, so it should not be in temp memory
// if function is NULL, the command will be forwarded to the server
// as a clc_clientCommand instead of executed locally

void	Cmd_RemoveCommand(const char *cmd_name);

void	Cmd_CommandCompletion(void(*callback)(const char *s));
// callback with each valid string

int		Cmd_Argc(void);
char	*Cmd_Argv(int arg);
void	Cmd_ArgvBuffer(int arg, char *buffer, int bufferLength);
char	*Cmd_Args(void);
char	*Cmd_ArgsFrom(int arg);
void	Cmd_ArgsBuffer(char *buffer, int bufferLength);
char	*Cmd_Cmd(void);
// The functions that execute commands get their parameters with these
// functions. Cmd_Argv () will return an empty string, not a NULL
// if arg > argc, so string operations are allways safe.

void	Cmd_TokenizeString(const char *text);
// Takes a null terminated string.  Does not need to be /n terminated.
// breaks the string up into arg tokens.

void	Cmd_ExecuteString(const char *text);
// Parses a single line of text into arguments and tries to execute it
// as if it was typed at the console
#endif // !CMD_H_

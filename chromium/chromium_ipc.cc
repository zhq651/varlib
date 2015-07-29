/* add the follow file we can use the message dispatch {{{ */

#include "chrome/common/ipc_message.cc"
#include "chrome/common/ipc_sync_message.cc"
/* }}} add the follow file we can use the message dispatch */

#include "base/stats_table.cc"
#include "base/waitable_event_watcher_win.cc"
#include "base/process_util_win.cc"
#include "base/shared_memory_win.cc"
#include "base/revocable_store.cc"
#include "chrome/common/chrome_counters.cc"
#include "chrome/common/ipc_channel_win.cc"
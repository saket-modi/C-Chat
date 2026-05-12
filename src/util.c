#include "util.h"
Message compose(char* txt) {
    Message msg;
    msg.msg = txt;
    msg.len = strlen(txt);
    return msg;
}
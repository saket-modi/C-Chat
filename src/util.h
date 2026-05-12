typedef struct {
    int len;
    char msg[BUFSIZ];
} Message;

Message compose(char*);
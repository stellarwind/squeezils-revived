void Node::Handle(void *arg)
{
    if (gCallback) {
        if (gCallback->enabled) {
            if (strcmp(name, kName) == 0)
                gCallback->Notify(this, arg);
            else
                fprintf(stderr, kFmt, name);
        }
    } else
        Missing(kMissing);
}

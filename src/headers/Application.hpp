#ifndef APPLICATION_H
#define APPLICATION_H

class Application
{
public:
    Application(char** argv, int argc);

    void RunTooling();
    void Build();

private:
    char** argv;
    int    argc;
};
#endif

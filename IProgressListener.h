#ifndef I_PROGRESS_LISTENER_H
#define I_PROGRESS_LISTENER_H

class IProgressListener
{
public:
    virtual void UpdateProgress(short Perc) = 0;
};

#endif // I_PROGRESS_LISTENER_H

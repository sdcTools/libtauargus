#ifndef I_PROGRESS_LISTENER_H
#define I_PROGRESS_LISTENER_H

class IProgressListener
{
public:
	virtual ~IProgressListener() {}
    virtual void UpdateProgress(int Perc) = 0;
};

#endif // I_PROGRESS_LISTENER_H

#ifndef I_ProgressListener_h
#define I_ProgressListener_h

class IProgressListener
{
public:
	virtual ~IProgressListener() { }
    virtual void UpdateProgress(int Perc) = 0;
};

#endif // I_ProgressListener_h

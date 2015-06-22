/*
** Pure virtual class to notify an object that an image window was raised
*/
#ifndef __PNotifyRaised_h__
#define __PNotifyRaised_h__

class PImageWindow;

class PNotifyRaised {
public:
	PNotifyRaised();
	virtual ~PNotifyRaised();
	
	virtual void	NotifyRaised(PImageWindow *aWindow)	= 0;

	void			ArmNotifyRaised();
};

#endif // __PNotifyRaised_h__

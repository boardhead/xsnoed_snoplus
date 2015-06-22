#include "PNotifyRaised.h"
#include "PImageWindow.h"

PNotifyRaised::PNotifyRaised()
{
}

PNotifyRaised::~PNotifyRaised()
{
	if (PImageWindow::sNotifyRaised == this) {
		PImageWindow::sNotifyRaised = NULL;
	}
}

void PNotifyRaised::ArmNotifyRaised()
{
	PImageWindow::sNotifyRaised = this;
}


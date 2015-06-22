/*
 * File:		messages.h
 *
 * Description:	messages sent by xsnoed objects
 *
 * Created:		12/04/99 - P. Harvey
 */
#ifndef __messages_h__
#define __messages_h__

enum {
	// null message
	kMessageNull = 0x0000,			// ignore this message
	
	// messages without data
	kMessageNewEvent = 0x0001,		// a new event was displayed
	kMessageEventCleared, 			// event was cleared
	kMessageColoursChanged,			// the hit color map has changed
	kMessageOpticalChanged,			// the optical constants changed
	kMessageSetToVertex,			// change display orientation to the event vertex
	kMessageSetToSun,				// change display orientation to the sun
	kMessageWorkProc,				// patch for work proc callback in ROOT version
	kMessageTriggerChanged,			// event trigger (continuous/single/none) changed
	kMessageSunMoved,				// the sun moved location
	kMessageVesselChanged,			// the vessel display changed
	kMessageGeometryChanged,		// main display geometry changed
	kMessageMonteCarloChanged,		// the monte carlo data changed
	kMessageHistoryChanged,			// the event history has changed
	kMessageHistoryEventChanged,	// the currently viewed event in history has changed
	kMessageHistoryChangeEnd,		// the history has finished changing
	kMessageHistoryWillClear,		// the history buffers are about to be cleared
	kMessageHitSizeChanged,			// the displayed hit size was changed
	kMessageNCDSizeChanged,			// the displayed NCD size was changed
	kMessageHitsChanged,			// the event hits changed
	kMessageHitLinesChanged,		// the hit lines setting was changed
	kMessageFitChanged,				// a displayed fit has changed
	kMessageFitLinesChanged,		// the fit lines setting was changed
	kMessageWaterChanged,			// the water level display changed
	kMessageGTIDFormatChanged,		// the GTID display format changed
	kMessageTimeFormatChanged,		// the format of the time display changed
	kMessageAngleFormatChanged,		// the angle display format changed
	kMessageLabelChanged,			// the image label changed
	kMessageNextTimeAvailable,		// the 'next' time is available for the viewed event
	kMessageShowLabelChanged,		// the state of 'show label' was changed
	kMessageWillWriteSettings,		// we are about to write our settings to file
	kMessageWriteSettingsDone,		// we are done writing our settings to file
	kMessageDataModeChanged,		// the data mode (event vs. CMOS rate) has changed
	kMessageHitXYZChanged,			// the hit XYZ setting has changed
	kMessageCalibrationChanged,		// the calibration type changed
	kMessageNewScopeData,           // new trigger scope data is available
	kMessagePMTDataTypeChanged,     // the displayed PMT data type changed
	kMessageNCDDataTypeChanged,     // the displayed NCD data type changed

	// messages from the global speaker (the resource manager)
	// (via PResourceManager::sSpeaker, not ImageData->mSpeaker)
	// - the difference is that all XSNOED main windows will hear these ones
	kMessageResourceColoursChanged = 0x1001,	// the colours changed
	kMessageResourceOpticalChanged,				// the optical parameters changed
	kMessageTranslationCallback,		// data is (TranslationData *)
	kMessageNewMainDisplayEvent,		// data is (PmtEventRecord *)	

	// messages with data
	kMessageMCVertexChanged	= 0x2001,	// data is (MonteCarloVertex *)
	kMessage3dCursorMotion,				// data is (PProjImage *)
	kMessageCursorHit,					// data is (PProjImage *)
	kMessageCursorNcd,					// data is (PProjImage *)
	kMessageHitDiscarded,				// data is (PProjImage *)
	kMessageNewHeaderRecord,			// data is (int *) RecHdr index
	
	kLastMessageID	// not used, but saves trying to remember about commas on the last enum
};

#endif // __messages_h__

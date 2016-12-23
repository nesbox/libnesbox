int FCEU_InitVirtualVideo(void);
extern uint8 *XBuf;
extern uint8 *XBackBuf;
extern int ClipSidesOffset;
extern struct GUIMESSAGE
{
	//countdown for gui messages
	int howlong;

	//the current gui message
	char errmsg[110];

	//indicates that the movie should be drawn even on top of movies
	bool isMovieMessage;

} guiMessage;

extern GUIMESSAGE subtitleMessage;

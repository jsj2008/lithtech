
#ifndef __DSTREAM_H__
#define __DSTREAM_H__

	
	// Helper macro to read/write data if your stream pointer is called pStream.
	#define STREAM_READ(_x_) pStream->Read(&(_x_), sizeof(_x_));
	#define STREAM_WRITE(_x_) pStream->Write(&(_x_), sizeof(_x_));
	

	class DStream
	{
	protected:
		
		virtual			~DStream() {}
	
	public:

		// Call this when you're done with it (same as deleting it).
		virtual void	Release()=0;

		// Read in data.  Returns LT_ERROR if you read past the end.  ALWAYS fills in
		// pData with 0's if it returns an error so you can safely read everything
		// in without checking the return value every time and check ErrorStatus() at the end.
		virtual DRESULT	Read(void *pData, unsigned long size)=0;
		virtual DRESULT ReadString(char *pStr, unsigned long maxBytes);
		
		// LT_OK = no errors, otherwise an error occured.
		virtual DRESULT	ErrorStatus()=0;
		
		virtual DRESULT	SeekTo(unsigned long offset)=0;
		virtual DRESULT	GetPos(unsigned long *offset)=0;
		virtual DRESULT	GetLen(unsigned long *len)=0;

		// Helpers to make it easy..
		unsigned long GetPos()
		{
			unsigned long pos;
			GetPos(&pos);
			return pos;
		}

		unsigned long GetLen()
		{
			unsigned long len;
			GetLen(&len);
			return len;
		}

		template<class T>
		DStream&	operator>>(T &toRead) {Read(&toRead, sizeof(toRead)); return *this;}

	// Only enabled in writeable streams.
	public:
		
		virtual DRESULT	Write(void *pData, unsigned long size)=0;
		virtual DRESULT WriteString(char *pStr);

		template<class T>
		DStream&	operator<<(T toWrite) {Write(&toWrite, sizeof(toWrite)); return *this;}
	};


#endif  // __DSTREAM_H__




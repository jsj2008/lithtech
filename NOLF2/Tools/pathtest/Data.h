#ifndef __DATA_H__
#define __DATA_H__

class CWorld;


class CData
{
public:

	 CData() {}
	~CData() {}

	virtual void Init(CWorld* pWorld) {}

protected:

};

class CData1 : public CData
{
public:
	virtual void Init(CWorld* pWorld);
};

class CData2 : public CData
{
public:
	virtual void Init(CWorld* pWorld);
};

class CData3 : public CData
{
public:
	virtual void Init(CWorld* pWorld);
};

#endif
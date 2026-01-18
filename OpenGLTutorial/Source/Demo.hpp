#pragma once

class Demo
{
protected:
	Demo() = default;

public:
	virtual ~Demo() = default;

	virtual void prepare() = 0;
	virtual void run() = 0;
	virtual bool shouldEnd() = 0;
	virtual void terminate() = 0;
};
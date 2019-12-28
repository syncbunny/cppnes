#ifndef LOG_H
#define LOG_H

class Logger {
public:
	enum Level {
		DEBUG = 0,
		INFO  = 1,
		WARN  = 2,
		ALERT = 3,
		ERROR = 4,
		NONE  = 999,
	};
protected:
	Logger();
public:
	virtual ~Logger();

public:
	static Logger* getInstance() {
		static Logger* instance = 0;

		if (instance == 0) {
			instance = new Logger();
		}

		return instance;
	}	

public:
	virtual void log(int lvl, const char* fmt, ...);

protected:
	int mLevel;
};

#endif

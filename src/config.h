#ifndef CONFIG_H
#define CONFIG_H

#include <string>

class Config {
protected:
	Config();
public:
	~Config();

public:
	static Config* getInstance() {
		static Config* instance = 0;

		if (instance == 0) {
			instance = new Config();
		}
		return instance;
	}

public:
	void setVarbose(bool v) {
		mVarbose = v;
	}
	bool getVarbose() const {
		return mVarbose;
	}

	void setROMPath(const std::string& path) {
		mROMPath = path;
	}
	std::string getROMPath() const {
		return mROMPath;
	}

	void setCorePath(const std::string& path) {
		mCorePath = path;
	}
	std::string getCorePath() const {
		return mCorePath;
	}

protected:
	bool mVarbose;
	std::string mROMPath;
	std::string mCorePath;
};

#endif

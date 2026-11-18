class OutStream {
	std::ofstream stream;

public:
	OutStream(std::string path);
	~OutStream();

	void write(const char* str, size_t size);

	template <typename T> void operator<<(T& val) {
		stream.write(reinterpret_cast<char*>(&val), sizeof(T));
	}

	void operator<<(std::string& val);
	void operator<<(std::unordered_map<int, int>& val);
	void operator<<(std::unordered_set<int>& val);
	void operator<<(cocos2d::CCPoint& val);
};

class InStream {
	std::ifstream stream;

public:
	InStream(std::string path);
	~InStream();

	void read(char* str, size_t size);
	void ignore(std::streamsize count);

	template <typename T> void operator>>(T& val) {
		stream.read(reinterpret_cast<char*>(&val), sizeof(T));
	}

	void operator>>(std::string& val);
	void operator>>(std::unordered_map<int, int>& val);
	void operator>>(std::unordered_set<int>& val);
	void operator>>(cocos2d::CCPoint& val);
};

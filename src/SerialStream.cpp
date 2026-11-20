#include "SerialStream.hpp"

#include <asp/fs/std.hpp>

#define SWRITE(val)                                                            \
	{                                                                           \
		auto __val = val;                                                        \
		stream.write(reinterpret_cast<char*>(&__val), sizeof(__val));            \
	}
#define SREAD(val) stream.read(reinterpret_cast<char*>(&val), sizeof(val))

OutStream::OutStream(asp::fs::Path path) {
	stream.open(path.toStdPath().unwrap(), std::ios::binary | std::ios::trunc);
}

InStream::InStream(asp::fs::Path path) {
	stream.open(path.toStdPath().unwrap(), std::ios::binary);
}

OutStream::~OutStream() { stream.close(); }

InStream::~InStream() { stream.close(); }

void OutStream::write(const char* str, size_t size) { stream.write(str, size); }

void InStream::read(char* str, size_t size) { stream.read(str, size); }

void InStream::ignore(std::streamsize count) { stream.ignore(count); }

void OutStream::operator<<(std::string& txt) {
	unsigned int size = txt.size();
	SWRITE(size);
	if (size > 0)
		stream.write(txt.data(), size);
}

void InStream::operator>>(std::string& txt) {
	unsigned int size;
	SREAD(size);
	if (size == 0)
		return;
	char* buf = new char[size + 1];
	stream.read(buf, size);
	buf[size] = '\0';
	txt = buf;
	delete[] buf;
}

void OutStream::operator<<(std::unordered_map<int, int>& map) {
	SWRITE(map.size());
	for (std::pair<int, int> e : map) {
		SWRITE(e.first);
		SWRITE(e.second);
	}
}

void InStream::operator>>(std::unordered_map<int, int>& map) {
	size_t size;
	SREAD(size);

	for (size_t i = 0; i < size; ++i) {
		int key;
		int value;
		SREAD(key);
		SREAD(value);
		map[key] = value;
	}
}

void OutStream::operator<<(std::unordered_set<int>& set) {
	SWRITE(set.size());
	for (int e : set) {
		SWRITE(e);
	}
}

void InStream::operator>>(std::unordered_set<int>& set) {
	size_t size;
	SREAD(size);

	for (size_t i = 0; i < size; ++i) {
		int value;
		SREAD(value);
		set.insert(value);
	}
}

void OutStream::operator<<(cocos2d::CCPoint& p) {
	SWRITE(p.x);
	SWRITE(p.y);
}

void InStream::operator>>(cocos2d::CCPoint& p) {
	SREAD(p.x);
	SREAD(p.y);
}

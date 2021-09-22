template <class T>
inline void ReadDesc(T *t, std::istream &in) {
	in.read((char*)t, sizeof(T));
}

template <class T>
static inline void WriteDesc(T *t, std::ostream &out) {
	out.write((char*)t, sizeof(T));
}

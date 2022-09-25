#include <string>

int str_to_int(const std::string& s) {
	return std::stoi(s.c_str());
}
float str_to_float(const std::string& s) {
	return std::stof(s.c_str());
}
bool str_to_bool(const std::string& s) {
	return s=="true";
}
std::string int_to_str(int i){
	char buf[16];
	snprintf(buf,sizeof(buf), "%d",i);
	return std::string(buf);
}
void replace_str(std::string& str, std::string oldstr, std::string newstr) {
	size_t cur_pos	= 0;
	size_t find_pos = std::string::npos;
	while ((find_pos = str.find(oldstr, cur_pos)) != std::string::npos) {
		str.replace(find_pos, oldstr.size(), newstr);
		cur_pos = find_pos;
	}
}

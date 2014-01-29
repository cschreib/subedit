#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>

void print() {
    std::cout << std::endl;
}

template<typename T, typename ... Args>
void print(const T& t, const Args& ... args) {
    std::cout << t;
    print(args...);
}

template<typename ... Args>
void error(const Args& ... args) {
    std::cout << "error: ";
    print(args...);
}

template<typename ... Args>
void warning(const Args& ... args) {
    std::cout << "warning: ";
    print(args...);
}

template<typename ... Args>
void note(const Args& ... args) {
    std::cout << "note: ";
    print(args...);
}

void put() {
    std::cout << std::flush;
}

template<typename T, typename ... Args>
void put(const T& t, const Args& ... args) {
    std::cout << t;
    put(args...);
}

namespace string {
    template<typename T>
    bool from_string(const std::string& s, T& t) {
        std::istringstream ss(s);
        return (ss >> t);
    }

    std::string to_lower(std::string s) {
        for (auto& c : s) {
            c = ::tolower(c);
        }
        return s;
    }

    std::string erase_start(std::string s, std::size_t n) {
        if (n >= s.size()) {
            return "";
        }

        s.erase(0, n);
        return s;
    }

    template<typename T>
    std::vector<std::string> cut(const std::string& ts, const T& pattern) {
        std::vector<std::string> ret;
        std::size_t p = 0, op = 0;
        while ((p = ts.find(pattern, op)) != ts.npos) {
            ret.push_back(ts.substr(op, p - op));
            op = p+1;
        }

        ret.push_back(ts.substr(op));

        return ret;
    }

    template<typename T>
    std::string convert(const T& t) {
        std::ostringstream ss;
        ss << t;
        return ss.str();
    }

    template<typename T>
    std::string convert(const T& t, std::size_t n, char fill = '0') {
        if (n <= 1) {
            return convert(t);
        }
        if (t == 0) {
            return std::string(n-1, fill)+'0';
        } else {
            std::ostringstream ss;
            ss << t;
            std::size_t nz = n-1 - floor(log10(t));
            if (nz > 0 && nz < 6) {
                return std::string(nz, fill) + ss.str();
            } else {
                return ss.str();
            }
        }
    }
}

struct time_key {
    int seconds;
    int milliseconds;

    time_key() : seconds(-1), milliseconds(-1) {}

    time_key(float sec) : seconds(floor(sec)), milliseconds((sec - floor(sec))*1000) {}

    explicit time_key(const std::string& str) : seconds(0), milliseconds(0) {
        std::vector<std::string> words = string::cut(str, ":");
        std::vector<std::string>::iterator iter = words.begin();
        int tmp = 0;

        if (words.size() == 3) {
            if (!string::from_string(*iter, tmp)) {
                error("invalid number of hours\n");
                seconds = milliseconds = -1;
                return;
            } else {
                seconds += tmp*3600;
            }

            ++iter;
        }

        if (words.size() >= 2) {
            if (!string::from_string(*iter, tmp)) {
                error("invalid number of minutes\n");
                seconds = milliseconds = -1;
                return;
            } else {
                seconds += tmp*60;
            }

            ++iter;
        }

        words = string::cut(*iter, ",");
        if (!string::from_string(words.front(), tmp)) {
            error("invalid number of seconds\n");
            seconds = milliseconds = -1;
            return;
        } else {
            seconds += tmp;
        }

        if (words.size() == 2) {
            if (!string::from_string(words.back(), milliseconds)) {
                error("invalid number of milliseconds\n");
                seconds = milliseconds = -1;
                return;
            }
        }
    }

    time_key(int sec, int msec) : seconds(sec), milliseconds(msec) {
        while (milliseconds >= 1000) {
            ++seconds;
            milliseconds -= 1000;
        }
        while (milliseconds < 0) {
            --seconds;
            milliseconds += 1000;
        }
    }

    bool valid() const {
        return seconds != -1 && milliseconds != -1;
    }

    time_key operator + (const time_key& t) const {
        return time_key(seconds + t.seconds, milliseconds + t.milliseconds);
    }

    time_key& operator += (const time_key& t) {
        seconds += t.seconds;
        milliseconds += t.milliseconds;

        while (milliseconds >= 1000) {
            ++seconds;
            milliseconds -= 1000;
        }
        while (milliseconds < 0) {
            --seconds;
            milliseconds += 1000;
        }

        return *this;
    }

    time_key operator - (const time_key& t) const {
        return time_key(seconds - t.seconds, milliseconds - t.milliseconds);
    }

    time_key& operator -= (const time_key& t) {
        seconds -= t.seconds;
        milliseconds -= t.milliseconds;

        while (milliseconds >= 1000) {
            ++seconds;
            milliseconds -= 1000;
        }
        while (milliseconds < 0) {
            --seconds;
            milliseconds += 1000;
        }

        return *this;
    }

    bool operator < (const time_key& t) const {
        if (seconds == t.seconds) {
            return (milliseconds < t.milliseconds);
        } else {
            return (seconds < t.seconds);
        }
    }

    bool operator <= (const time_key& t) const {
        if (seconds == t.seconds) {
            return (milliseconds <= t.milliseconds);
        } else {
            return (seconds <= t.seconds);
        }
    }

    bool operator > (const time_key& t) const {
        if (seconds == t.seconds) {
            return (milliseconds > t.milliseconds);
        } else {
            return (seconds > t.seconds);
        }
    }

    bool operator >= (const time_key& t) const {
        if (seconds == t.seconds) {
            return (milliseconds >= t.milliseconds);
        } else {
            return (seconds >= t.seconds);
        }
    }
};

std::ostream& operator << (std::ostream& o, const time_key& t) {
    int hour = t.seconds/3600;
    int min = (t.seconds - hour*3600)/60;
    int sec = (t.seconds - hour*3600 - min*60);

    return o << string::convert(hour, 2) << ":" << string::convert(min, 2)
        << ":" << string::convert(sec, 2) << "," << string::convert(t.milliseconds, 3);
}

struct entry {
    std::size_t id;
    time_key start;
    time_key end;
    std::string content;
};

std::ostream& operator << (std::ostream& o, const entry& e) {
    return o << e.id << "\n" << e.start << " --> " << e.end << "\n" << e.content;
}

bool find_next(std::vector<entry>& array, std::vector<entry>::iterator& iter,
    const std::string& str) {

    while (iter != array.end()) {
        if (iter->content.find(str)) {
            return true;
        }

        ++iter;
    }

    return false;
}

bool find_previous(std::vector<entry>& array, std::vector<entry>::iterator& iter,
    const std::string& str) {

    while (iter != array.begin()) {
        --iter;

        if (iter->content.find(str)) {
            return true;
        }
    }

    return false;
}

void print_help() {
    print("\nsubedit v1.0\n");

    print("  This program can do some basic editing on subtitles.");
    print("  Its only function for now it to be able to add a delay to a");
    print("  particular sentence, shifting all the following ones.");
    print("  To do so, you must first pick the sentence you want to shift");
    print("  (using any of the various possibilities discribed below),");
    print("  then press 'Enter' to input the amount of seconds you want");
    print("  to add/remove. The program will update the subtitle accordingly");
    print("  and save it right away, so you can immediately check the result");
    print("  in your favorite movie player.");
    print("  Note that some players require a restart for the changes to be");
    print("  effectives (VLC for example).\n");

    print("Available search commands:");
    print("  hh:mm:ss,mili : select the sentence just after the provided time stamp");
    print("  any text      : search for occurences of 'any text', enters search mode");
    print("  ?             : exit search mode");
    print("  #n            : select the 'n'th sentence ('n'th occurence in search mode)");
    print("  +x            : advance 'x' times (next occurences in search mode)");
    print("  +             : advance once (next occurence in search mode)");
    print("  -x            : step back 'x' times (previous occurences in search mode)");
    print("  -             : step back once (previous occurence in search mode)\n");

    print("Other commands:");
    print("  'empty'       : starts editing the current sentence's time stamp");
    print("  help or h     : displays this text");
    print("  quit or q     : exits the program\n");
}

int main(int argc, char* argv[]) {
    std::vector<entry> entries;
    std::string file_name;

    if (argc > 1) {
        file_name = argv[argc-1];
        std::ifstream file(file_name);

        if (!file.is_open()) {
            error("cannot open file: "+file_name+".");
            return 1;
        }

        entry e;
        std::size_t count = 0;
        std::size_t l = 0;
        while (!file.eof()) {
            ++l;
            std::string line;
            getline(file, line);

            if (!line.empty()) {
                if (count == 0) {
                    if (!string::from_string(line, e.id)) {
                        error(l, ": bad entry ID");
                        return 1;
                    }
                } else if (count == 1) {
                    std::vector<std::string> words = string::cut(line, " --> ");
                    if (words.size() == 2) {
                        e.start = time_key(words.front());
                        e.end = time_key(words.back());
                    } else {
                        error(l, ": bad time tag format (expected <time1> --> <time2>)");
                        return 1;
                    }
                } else {
                    e.content += line + "\n";
                }
                ++count;
            } else if (count != 0) {
                entries.push_back(e);
                e.content = "";
                count = 0;
            }
        }

        file.close();

        print("subtitle successfully loaded!");
    } else {
        print_help();
        return 0;
    }

    note("if you need help, type 'help' or 'h'. Type 'q' to exit.\n");

    bool no_display = true;
    bool search_mode = false;
    std::string search_string = "";
    time_key key;
    std::vector<entry>::iterator iter = entries.begin();

    while (true) {
        if (!no_display) {
            if (iter != entries.end()) {
                print("\n[", iter - entries.begin(), "] ", iter->start, " :\n\n", iter->content);
            }
        }

        no_display = false;

        put("> ");

        std::string s;
        getline(std::cin, s);
        std::string low = string::to_lower(s);

        if (s.empty()) {
            no_display = true;
            print("\nhow many seconds do you want to add / remove (empty to abord)? ");

            float sec = 0.0f;
            bool stop = false;
            while (true) {
                getline(std::cin, s);

                if (s.empty()) {
                    stop = true;
                    break;
                }

                if (!string::from_string(s, sec)) {
                    error("invalid time duration, please enter a floating point number (or nothing "
                        "to abort): ");
                } else {
                    break;
                }
            }

            if (stop) {
                print("");
                continue;
            }

            put("editing subtitle, please wait... ");

            std::size_t count = 0;
            while (iter != entries.end()) {
                iter->start += sec;
                iter->end += sec;
                ++iter;
                ++count;
            }

            put("done (", count, " entries modified).\nSaving... ");

            std::ofstream file(file_name);

            for (auto& e : entries) {
                file << e;
            }

            file.close();

            print(" done.");
            continue;
        } else if (low == "q" || low == "quit") {
            return 0;
        } else if (low == "h" || low == "help") {
            print_help();
            no_display = true;
            continue;
        } else {
            char c = s[0];
            if (c == '#') {
                s = string::erase_start(s, 1);
                std::size_t num = 0;
                if (!string::from_string(s, num)) {
                    error("invalid number of entry\n");
                    no_display = true;
                    continue;
                }

                if (!search_mode) {
                    std::vector<entry>::iterator old = iter;
                    iter = entries.begin();
                    std::size_t i = num;
                    while (iter != entries.end() && i != 0) {
                        --i;
                        ++iter;
                    }

                    if (iter == entries.end()) {
                        error("not enough entries (max : ", entries.size()-1, ")\n");
                        no_display = true;
                        iter = old;
                        continue;
                    }

                    key = iter->start;
                } else {
                    ++num;
                    std::vector<entry>::iterator old = iter;
                    iter = entries.begin();
                    bool res = find_next(entries, iter, search_string);
                    --num;

                    while (res && num != 0) {
                        old = iter;
                        ++iter;
                        res = find_next(entries, iter, search_string);
                        --num;
                    }

                    if (!res) {
                        error("no further matches, displaying last one\n");
                        iter = old;
                    }

                    key = iter->start;
                }
            } else if (c == '?') {
                search_string = string::erase_start(s, 1);

                if (search_string.empty()) {
                    search_mode = false;
                    no_display = true;
                    continue;
                }

                std::vector<entry>::iterator old = iter;
                bool res = find_next(entries, iter, search_string);
                if (!res) {
                    iter = entries.begin();
                    res = find_next(entries, iter, search_string);

                    if (!res) {
                        error("no match for '"+search_string+"'\n");
                        no_display = true;
                        iter = old;
                        key = iter->start;
                        continue;
                    }
                }

                search_mode = true;
            } else if (c == '+') {
                std::size_t num;

                if (s.size() == 1) {
                    num = 1;
                } else {
                    s = string::erase_start(s, 1);
                    if (!string::from_string(s, num)) {
                        error("invalid number of entries\n");
                        no_display = true;
                        continue;
                    }
                }

                if (!search_mode) {
                    std::size_t i = num;
                    while (iter != entries.end() && i != 0) {
                        ++iter;
                        --i;
                    }

                    if (iter == entries.end()) {
                        --iter;
                        if (num == 1) {
                            error("no further entry\n");
                            no_display = true;
                        } else {
                            error("only ", num-i, " further entries, displaying last one\n");
                            key = iter->start;
                        }

                        continue;
                    }

                    key = iter->start;
                } else {
                    std::vector<entry>::iterator old = iter;
                    bool res = true;
                    std::size_t i = num;
                    while (res && i != 0) {
                        old = iter;
                        ++iter;
                        res = find_next(entries, iter, search_string);
                        --i;
                    }

                    if (!res) {
                        iter = old;

                        if (num == 1) {
                            error("no further matches\n");
                            no_display = true;
                            continue;
                        } else {
                            error("only ", num-i, " further matches, displaying last one\n");
                        }
                    }

                    key = iter->start;
                }
            } else if (c == '-') {
                std::size_t num;

                if (s.size() == 1) {
                    num = 1;
                } else {
                    s = string::erase_start(s, 1);
                    if (!string::from_string(s, num)) {
                        error("invalid number of entries\n");
                        no_display = true;
                        continue;
                    }
                }

                if (!search_mode) {
                    std::vector<entry>::iterator old = iter;
                    std::size_t i = num;
                    while (iter != entries.begin() && i != 0) {
                        --iter;
                        --i;
                    }

                    if (iter == entries.begin() && i != 0) {
                        iter = old;

                        if (num == 1) {
                            error("no entry before this point\n");
                            no_display = true;
                            continue;
                        } else {
                            error("only ", num-i, " entries before this point, displaying first "
                                "one\n");
                        }
                    }

                    key = iter->start;
                } else {
                    std::vector<entry>::iterator old = iter;
                    bool res = true;
                    std::size_t i = num;
                    while (res && i != 0) {
                        old = iter;
                        res = find_previous(entries, iter, search_string);
                        --i;
                    }

                    if (!res) {
                        if (num == 1) {
                            error("no match before this point\n");
                            no_display = true;
                            iter = old;
                        } else {
                            error("only ", num-i, " matches before this point, displaying first "
                                "one\n");
                            key = iter->start;
                        }

                        continue;
                    }
                }
            } else {
                time_key tmp = time_key(s);
                if (tmp.valid()) {
                    std::vector<entry>::iterator old = iter;

                    for (auto& e : entries) {
                        if (e.start >= tmp) {
                            break;
                        }
                    }

                    if (iter == entries.end()) {
                        error("no entry after ", tmp, "\n");
                        no_display = true;
                        iter = old;
                        continue;
                    }

                    key = iter->start;
                } else {
                    search_string = s;

                    std::vector<entry>::iterator old = iter;
                    bool res = find_next(entries, iter, search_string);
                    if (!res) {
                        iter = entries.begin();
                        res = find_next(entries, iter, search_string);

                        if (!res) {
                            error("no match for '"+search_string+"'\n");
                            no_display = true;
                            iter = old;
                            key = iter->start;
                            continue;
                        }
                    }

                    search_mode = true;
                }
            }
        }
    }
}

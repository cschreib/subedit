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

    std::string trim(std::string s, const std::string& chars = " \t") {
        std::size_t spos = s.find_first_of(chars);
        if (spos == 0) {
            std::size_t epos = s.find_first_not_of(chars);
            if (epos == s.npos) return "";
            s = s.substr(epos);
        }

        spos = s.find_last_of(chars);
        if (spos == s.size()-1) {
            std::size_t epos = s.find_last_not_of(chars);
            s = s.erase(epos+1, s.size() - epos+1);
        }

        return s;
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

    std::vector<std::string> cut(const std::string& ts, const std::string& pattern) {
        std::vector<std::string> ret;
        std::size_t p = 0, op = 0;
        while ((p = ts.find(pattern, op)) != ts.npos) {
            ret.push_back(ts.substr(op, p - op));
            op = p + pattern.size();
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
    time_key() : seconds(-1), milliseconds(-1) {}

    explicit time_key(float sec) : seconds(floor(sec)), milliseconds((sec - floor(sec))*1000) {}

    time_key(const std::string& str, std::string& err) : seconds(0), milliseconds(0) {
        std::vector<std::string> words = string::cut(string::trim(str), ":");
        std::vector<std::string>::iterator iter = words.begin();
        int tmp = 0;

        if (words.size() == 3) {
            if (!string::from_string(*iter, tmp)) {
                err = "invalid number of hours";
                seconds = milliseconds = -1;
                return;
            } else {
                seconds += tmp*3600;
            }

            ++iter;
        }

        if (words.size() >= 2) {
            if (!string::from_string(*iter, tmp)) {
                err = "invalid number of minutes";
                seconds = milliseconds = -1;
                return;
            } else {
                seconds += tmp*60;
            }

            ++iter;
        }

        words = string::cut(*iter, ",");
        if (!string::from_string(words.front(), tmp)) {
            err = "invalid number of seconds";
            seconds = milliseconds = -1;
            return;
        } else {
            seconds += tmp;
        }

        if (words.size() == 2) {
            if (!string::from_string(words.back(), milliseconds)) {
                err = "invalid number of milliseconds";
                seconds = milliseconds = -1;
                return;
            }
        }

        normalize_();
    }

    time_key(int sec, int msec) : seconds(sec), milliseconds(msec) {
        normalize_();
    }

    bool valid() const {
        return seconds != -1 && milliseconds != -1;
    }

    time_key& operator += (float sec) {
        seconds += floor(sec);
        milliseconds += (sec - floor(sec))*1000;
        normalize_();
        return *this;
    }

    time_key& operator -= (float sec) {
        seconds -= floor(sec);
        milliseconds -= (sec - floor(sec))*1000;
        normalize_();
        return *this;
    }

    time_key operator + (float sec) const {
        time_key tmp(*this);
        tmp += sec;
        return tmp;
    }

    time_key operator - (float sec) const {
        time_key tmp(*this);
        tmp -= sec;
        return tmp;
    }

    float operator - (const time_key& t) const {
        int sec = seconds - t.seconds;
        int msec = milliseconds - t.milliseconds;
        normalize_(sec, msec);
        return sec + msec/1000.0f;
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

private :
    int seconds;
    int milliseconds;

    static void normalize_(int& sec, int& msec) {
        while (msec >= 1000) {
            ++sec;
            msec -= 1000;
        }
        while (msec < 0) {
            --sec;
            msec += 1000;
        }
    }

    void normalize_() {
        if (!valid()) return;
        normalize_(seconds, milliseconds);
    }

    friend std::ostream& operator << (std::ostream& o, const time_key& t) {
        int hour = t.seconds/3600;
        int min = (t.seconds - hour*3600)/60;
        int sec = (t.seconds - hour*3600 - min*60);

        return o << string::convert(hour, 2) << ":" << string::convert(min, 2)
            << ":" << string::convert(sec, 2) << "," << string::convert(t.milliseconds, 3);
    }
};

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
        if (iter->content.find(str) != std::string::npos) {
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

        if (iter->content.find(str) != std::string::npos) {
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
    print("  any text      : search for the first occurence of 'any text' and enters search mode");
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
                line = string::trim(line);
                if (count == 0) {
                    if (!string::from_string(line, e.id)) {
                        error(l, ": bad entry ID");
                        return 1;
                    }
                } else if (count == 1) {
                    std::vector<std::string> words = string::cut(line, " --> ");
                    if (words.size() == 2) {
                        std::string err;
                        e.start = time_key(words.front(), err);
                        if (!e.start.valid()) {
                            error(err);
                            note("parsing l.", l, " start time (", words.front(), ")");
                        }

                        err.clear();
                        e.end = time_key(words.back(), err);
                        if (!e.end.valid()) {
                            error(err);
                            note("parsing l.", l, " end time (", words.back(), ")");
                        }
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
        s = string::trim(s);
        std::string low = string::to_lower(s);

        if (s.empty()) {
            no_display = true;
            put("\ncorrected time (empty to abord)? ");

            float sec = 0.0f;
            bool stop = false;
            while (true) {
                getline(std::cin, s);

                if (s.empty()) {
                    stop = true;
                    break;
                }

                s = string::trim(s);
                if (s[0] == '+' || s[0] == '-') {
                    if (!string::from_string(s, sec)) {
                        error("invalid time duration, please enter a time stamp, a number, or nothing "
                            "to abort): ");
                    } else {
                        break;
                    }
                } else {
                    std::string err;
                    time_key tmp(s, err);
                    if (!tmp.valid()) {
                        error(err, ", please enter a time stamp, a number, or nothing "
                            "to abort): ");
                    } else {
                        sec = tmp - iter->start;
                        note("shifting by ", (sec > 0 ? "+" : ""), sec, " seconds");
                        break;
                    }
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

            put("done (", count, " entries modified).\nsaving... ");

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
                if (search_mode) {
                    search_mode = false;
                    no_display = true;
                    note("leaving search mode");
                } else {
                    no_display = true;
                    error("you are not in search mode");
                }
                continue;
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
                std::string err;
                time_key tmp = time_key(s, err);
                if (tmp.valid()) {
                    std::vector<entry>::iterator old = iter;

                    iter = entries.begin();
                    while (iter != entries.end()) {
                        if (iter->start >= tmp) {
                            break;
                        }
                        ++iter;
                    }

                    if (iter == entries.end()) {
                        error("no entry after ", tmp, "\n");
                        no_display = true;
                        iter = old;
                        continue;
                    }

                    key = iter->start;
                } else {
                    search_string = string::trim(string::trim(s), "\"\'");
                    if (!search_string.empty()) {
                        note("entering search mode for '", search_string, "'");
                        std::vector<entry>::iterator old = iter;
                        bool res = find_next(entries, iter, search_string);
                        if (!res) {
                            note("no further match from this point, starting over from begining");
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
}

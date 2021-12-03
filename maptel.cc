#include <unordered_map>
#include <unordered_set>
#include <cassert>
#include "maptel.h"

#ifdef NDEBUG
const bool debug = false;
#else
const bool debug = true;
#endif

namespace {
    const char FIRST_VALID_CHARACTER = '0';
    const char LAST_VALID_CHARACTER = '9';

    size_t idCount = 0;

    typedef std::unordered_map<unsigned long,
            std::unordered_map<std::string, std::string>> telDict;

    telDict &telMap() {
        static telDict telmap;
        return telmap;
    }

    // Konwertuje char const *s do napisu typu string.
    // Sprawdza przy tym poprawność numeru telefonu pod *s.
    std::string tel_num_to_string(char const *s) {
        std::string ans;
        [[maybe_unused]] bool terminalZeroReached = false;
        if (debug)
            assert(*s != '\0'); // Pusty numer jest niepoprawny.
        for (size_t i = 0; i < jnp1::TEL_NUM_MAX_LEN + 1; ++i) {
            ans += s[i];
            if (s[i] == '\0') {
                terminalZeroReached = true;
                break;
            }
            if (debug)
                assert(!(s[i] < FIRST_VALID_CHARACTER ||
                         s[i] > LAST_VALID_CHARACTER));
        }
        if (debug)
            assert(terminalZeroReached); // Sprawdza, dlugosc numeru.
        return ans;
    }

    // Zapisuje numer telefonu będący pod *tel_dst do napisu key.
    void set_tel_dst_value(char *tel_dst, std::string key) {
        for (size_t i = 0; i < key.size(); ++i)
            tel_dst[i] = key[i];
    }

    // W zależności od znalezienia ścieżki zmian numeru tel_src,
    // wywołuje odpowiednią funkcję do zapisania numeru tel_dst.
    void transform_tel_to_dst(unsigned long id, char *tel_dst,
                              [[maybe_unused]]size_t len,
                              std::string &key, std::string &firstKey) {
        if (telMap()[id].find(key) == telMap()[id].end()) {
            if (debug)
                assert(len >= key.length());
            set_tel_dst_value(tel_dst, key);
        } else {
            if (debug)
                assert(len > firstKey.length());
            set_tel_dst_value(tel_dst, firstKey);
            if (debug)
                std::cerr << "maptel: maptel_transform: cycle detected\n";
        }
    }
}

unsigned long jnp1::maptel_create(void) {
    if (debug)
        std::cerr << "maptel: maptel_create()" << '\n';
    telMap()[idCount];
    if (debug)
        std::cerr << "maptel: maptel_create: new map id = " << idCount << '\n';
    return idCount++;
}


void jnp1::maptel_delete(unsigned long id) {
    if (debug) {
        std::cerr << "maptel: maptel_delete(" << id << ")\n";
        assert(telMap().count(id) > 0);
    }
    telMap().erase(id);
    if (debug)
        std::cerr << "maptel: maptel_delete: map " << id << " deleted\n";
}

void jnp1::maptel_insert(unsigned long id,
                         char const *tel_src, char const *tel_dst) {
    if (debug) {
        std::cerr << "maptel: maptel_insert(" << id << ", " << tel_src << ", "
                  << tel_dst << ")\n";
        assert(tel_src != nullptr);
        assert(tel_dst != nullptr);
        assert(telMap().count(id) > 0);
    }

    std::string key = tel_num_to_string(tel_src);
    std::string destinationNumber = tel_num_to_string(tel_dst);
    telMap()[id][key] = destinationNumber;
    if (debug)
        std::cerr << "maptel: maptel_insert: inserted\n";
}

void jnp1::maptel_erase(unsigned long id, char const *tel_src) {
    if (debug) {
        std::cerr << "maptel: maptel_erase(" << id << ", " << tel_src
                  << ")\n";
        assert(tel_src != nullptr);
        assert(telMap().count(id) > 0);
    }
    std::string key = tel_num_to_string(tel_src);
    if (telMap()[id].count(key) == 0) {
        if (debug)
            std::cerr << "maptel: maptel_erase: nothing to erase\n";
    } else {
        telMap()[id].erase(key);
        if (debug)
            std::cerr << "maptel: maptel_erase: erased\n";
    }
}

void jnp1::maptel_transform(unsigned long id, char const *tel_src,
                            char *tel_dst, size_t len) {
    if (debug) {
        std::cerr << "maptel: maptel_transform(" << id << ", "
                  << tel_src << ", " << &tel_dst << ", " << len << ")\n";
        assert(tel_src != nullptr);
        assert(tel_dst != nullptr);
        assert(telMap().count(id) > 0);
    }

    std::unordered_set<std::string> visited;
    std::string key = tel_num_to_string(tel_src);
    std::string firstKey = key;

    while (telMap()[id].find(key) != telMap()[id].end() &&
           visited.find(key) == visited.end()) {
        visited.emplace(key);
        key = telMap()[id][key];
    }

    transform_tel_to_dst(id, tel_dst, len, key, firstKey);

    if (debug)
        std::cerr << "maptel: maptel_transform: " << tel_src
                  << " -> " << tel_dst << '\n';
}

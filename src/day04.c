// Exit the program with a given error message.
__attribute__((noreturn)) static void die(const char* message) {
  write(STDERR_FILENO, message, strlen(message));
  write(STDERR_FILENO, "\n", 1);
  exit(1);
}

// Print an integer in decimal, followed by a newline.
static void print_int(int x) {
  char buffer[16];
  buffer[15] = '\n';
  int i = 15;
  do {
    --i;
    buffer[i] = '0' + (x % 10);
    x /= 10;
  } while (x);
  write(STDOUT_FILENO, buffer + i, 16 - i);
}

static _Bool is_digit(char c) {
  return '0' <= c && c <= '9';
}

static _Bool is_lower(char c) {
  return 'a' <= c && c <= 'z';
}

static _Bool is_whitespace(char c) {
  return c == ' ' || c == '\n';
}

static _Bool is_hex(char c) {
  return is_digit(c) || ('a' <= c && c <= 'f');
}

// Read a decimal integer from the string at input into value, returning the
// address of the first byte after the integer.
static const char* read_int(const char* input, int* value) {
  if (!is_digit(*input)) return NULL;
  int temp = 0;
  while (is_digit(*input)) {
    temp = 10 * temp + (*input - '0');
    input++;
  }
  *value = temp;
  return input;
}

#define KEY3(a, b, c) ((unsigned)(a) | (unsigned)(b) << 8 | (unsigned)(c) << 16)
#define KEY(s) KEY3(s[0], s[1], s[2])
enum key {
  invalid = 0,
  byr = KEY3('b', 'y', 'r'),
  iyr = KEY3('i', 'y', 'r'),
  eyr = KEY3('e', 'y', 'r'),
  hgt = KEY3('h', 'g', 't'),
  hcl = KEY3('h', 'c', 'l'),
  ecl = KEY3('e', 'c', 'l'),
  pid = KEY3('p', 'i', 'd'),
  cid = KEY3('c', 'i', 'd'),
};

struct entry {
  enum key key;
  const char* value;
};

// Read a single passport entry into the provided struct. `key` and `value` will
// be updated to point at null-terminated strings for the key and value
// respectively. The `key` will always be of length 3, but the value can have
// arbitrary length.
static char* read_entry(char* i, struct entry* out) {
  const char* key = i;
  if (*i == '\n') {
    *out = (struct entry){0};
    return i + 1;
  }
  if (!is_lower(i[0]) || !is_lower(i[1]) || !is_lower(i[2]) || i[3] != ':') {
    die("syntax");
  }
  out->key = KEY(key);
  i += 4;
  out->value = i;
  while (!is_whitespace(*i)) i++;
  *i = '\0';
  return i + 1;
}

static char buffer[32768];

static _Bool check_year(const char* year, int min, int max) {
  if (strlen(year) != 4) return 0;
  int value;
  if (read_int(year, &value) != year + 4) return 0;
  return min <= value && value <= max;
}

static _Bool check_height(const char* height) {
  const int len = strlen(height);
  if (len < 3) return 0;
  const _Bool is_cm = height[len - 2] == 'c' && height[len - 1] == 'm';
  const _Bool is_in = height[len - 2] == 'i' && height[len - 1] == 'n';
  int value;
  if (read_int(height, &value) != height + len - 2) return 0;
  if (is_cm) {
    return 150 <= value && value <= 193;
  } else if (is_in) {
    return 59 <= value && value <= 76;
  } else {
    return 0;
  }
}

static _Bool check_hair(const char* color) {
  if (strlen(color) != 7) return 0;
  if (color[0] != '#') return 0;
  for (int i = 1; i < 7; i++) {
    if (!is_hex(color[i])) return 0;
  }
  return 1;
}

static _Bool check_eyes(const char* color) {
  if (strlen(color) != 3) return 0;
  switch (KEY(color)) {
    case KEY3('a', 'm', 'b'):
    case KEY3('b', 'l', 'u'):
    case KEY3('b', 'r', 'n'):
    case KEY3('g', 'r', 'y'):
    case KEY3('g', 'r', 'n'):
    case KEY3('h', 'z', 'l'):
    case KEY3('o', 't', 'h'):
      return 1;
    default:
      return 0;
  }
}

static _Bool check_pid(const char* value) {
  if (strlen(value) != 9) return 0;
  int x;
  return read_int(value, &x) == value + 9;
}

static _Bool check_entry(struct entry* e) {
  switch (e->key) {
    case byr: return check_year(e->value, 1920, 2002);
    case iyr: return check_year(e->value, 2010, 2020);
    case eyr: return check_year(e->value, 2020, 2030);
    case hgt: return check_height(e->value);
    case hcl: return check_hair(e->value);
    case ecl: return check_eyes(e->value);
    case pid: return check_pid(e->value);
    case cid: return 0;
    default: return 0;
  }
}

struct passport_validity {
  int part1;
  int part2;
};

static char* check_passport(
    char* i, char* const end, struct passport_validity* out) {
  int part1 = 0, part2 = 0;
  while (i < end) {
    struct entry entry;
    i = read_entry(i, &entry);
    if (!entry.key) break;
    part1 += entry.key != cid;
    part2 += check_entry(&entry);
  }
  out->part1 += part1 == 7;
  out->part2 += part2 == 7;
  return i;
}

int main() {
  // Parse the input.
  int len = read(STDIN_FILENO, buffer, sizeof(buffer));
  if (len < 0) die("read");
  char* i = buffer;
  char* const end = buffer + len;
  struct passport_validity num_valid = {0};
  while (i < end) {
    i = check_passport(i, end, &num_valid);
  }
  print_int(num_valid.part1);
  print_int(num_valid.part2);
}

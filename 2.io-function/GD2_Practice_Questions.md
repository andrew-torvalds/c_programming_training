# GĐ2 — Câu Hỏi Ôn Tập & Phỏng Vấn

> **Hướng dẫn:** Đọc câu hỏi, suy nghĩ kỹ, viết ra đáp án trước — rồi mới click **"Xem đáp án"** để kiểm tra.

---

## Mục Lục

- [Phần A — Lý Thuyết (Conceptual)](#phần-a--lý-thuyết-conceptual) — 20 câu
- [Phần B — Đọc Code (Code Reading)](#phần-b--đọc-code-code-reading) — 25 câu
- [Phần C — Viết Code (Coding)](#phần-c--viết-code-coding) — 15 câu

---

## Phần A — Lý Thuyết (Conceptual)

---

**A01.** `printf` không kiểm tra kiểu biến — điều này có nghĩa là gì? Cho ví dụ bug xảy ra do dùng sai format specifier.

<details>
<summary>Xem đáp án</summary>

`printf` chỉ đọc **bit pattern** từ argument theo format specifier bạn chỉ định — không quan tâm kiểu biến thực tế.

```c
int x = -1;
printf("%d\n", x);  // đọc 0xFFFFFFFF như signed   → -1
printf("%u\n", x);  // đọc 0xFFFFFFFF như unsigned  → 4294967295
```

**Bug thực tế:**
```c
uint32_t sensor_val = 4000000000;
printf("%d\n", sensor_val);  // in ra -294967296 — sai hoàn toàn!
// Fix: printf("%u\n", sensor_val);
```

Compiler thường warn: `format '%d' expects 'int' but argument is 'unsigned int'` — đây là lý do `-Wall` quan trọng.

</details>

---

**A02.** Tại sao `%zu` là specifier đúng cho `size_t`? Điều gì xảy ra nếu dùng `%d` thay vì `%zu` trên hệ thống 64-bit?

<details>
<summary>Xem đáp án</summary>

`size_t` là kiểu **unsigned**, kích thước phụ thuộc kiến trúc:
- 32-bit: `size_t` = 4 bytes
- 64-bit: `size_t` = 8 bytes

**Bug trên 64-bit với `%d`:**
```c
size_t n = sizeof(long);  // = 8 trên 64-bit Linux
printf("%d\n", n);        // %d chỉ đọc 4 bytes thấp → có thể in sai
printf("%zu\n", n);       // đúng — đọc đủ 8 bytes
```

Trên 64-bit, `%d` chỉ đọc 4 bytes trong khi `size_t` chiếm 8 bytes → đọc sai giá trị, thậm chí UB (undefined behavior) theo C standard vì kiểu không khớp.

</details>

---

**A03.** Giải thích `%02X` và `%08X`. Tại sao 2 format này phổ biến trong embedded hơn `%x` đơn thuần?

<details>
<summary>Xem đáp án</summary>

- `%02X`: tổng chiều rộng **tối thiểu 2 ký tự**, thiếu pad `0` phía trước, chữ **hoa**
- `%08X`: tổng chiều rộng **tối thiểu 8 ký tự**, thiếu pad `0` phía trước, chữ **hoa**

**Tại sao phổ biến trong embedded:**

```c
uint8_t  reg = 0x0B;
uint32_t addr = 0x40020000;

printf("%x\n",   reg);   // "b"          ← khó đọc, không rõ số bit
printf("%02X\n", reg);   // "0B"         ← rõ ràng, đúng 2 nibble
printf("%08X\n", addr);  // "40020000"   ← đúng 8 nibble, dễ map với datasheet
```

Khi debug register STM32/ESP32, datasheet luôn in địa chỉ dạng `0x40020000` — `%08X` match trực tiếp, không cần đoán số 0 đứng đầu.

</details>

---

**A04.** Giải thích cấu trúc IEEE 754 float 32-bit. Tại sao `0.1f` không thể biểu diễn chính xác trong bộ nhớ?

<details>
<summary>Xem đáp án</summary>

Float 32-bit chia thành 3 phần:
```
bit 31  |  bit 30-23  |  bit 22-0
  Sign  |  Exponent   |  Mantissa
  1 bit |   8 bit     |   23 bit
```

- **Sign:** `0` = dương, `1` = âm
- **Exponent:** số mũ, bias 127
- **Mantissa:** phần thập phân

**Tại sao `0.1f` không chính xác:**

23 bit mantissa chỉ biểu diễn được **2²³ ≈ 8 triệu** giá trị giữa mỗi lũy thừa 2. `0.1` trong binary là `0.000110011001100...` — vô hạn tuần hoàn, không thể lưu chính xác trong 23 bit. CPU lưu giá trị gần nhất:

```
0.1f thực tế = 0.10000000149011611938...
```

</details>

---

**A05.** Tại sao `0.1f + 0.2f == 0.3f` cho kết quả khác nhau trên x86 PC và MCU 32-bit? Giải thích cơ chế.

<details>
<summary>Xem đáp án</summary>

**MCU FPU 32-bit:**
```
0.1f = 0.100000001490...
0.2f = 0.200000002980...
0.1f + 0.2f = 0.300000004470  (tính thẳng 32-bit)
0.3f        = 0.300000011920
→ KHÔNG bằng
```

**x86 FPU 80-bit:**
- x86 load giá trị lên register 80-bit để tính
- `0.1f + 0.2f` tính với precision 80-bit → kết quả trung gian chính xác hơn
- Round về 32-bit → tình cờ ra `0.300000011920` — giống `0.3f`
- → BẰNG (nhưng chỉ là may mắn, không đáng tin)

**Kết luận:** Behavior phụ thuộc FPU precision — không portable, không đáng tin trên mọi nền tảng.

</details>

---

**A06.** Epsilon là gì? Tại sao không có một giá trị epsilon "đúng" cho mọi bài toán?

<details>
<summary>Xem đáp án</summary>

**Epsilon** là ngưỡng sai số chấp nhận được khi so sánh 2 số float:

```c
if (fabsf(a - b) < EPSILON) { /* "bằng nhau" */ }
```

**Tại sao không có epsilon "đúng" cho mọi bài toán:**

Epsilon phụ thuộc vào **độ chính xác yêu cầu của bài toán**:

| Bài toán | Epsilon |
|---|---|
| So sánh giá trị tính toán | `1e-6f` |
| Cảm biến nhiệt độ ±0.1°C | `0.01f` |
| Vị trí GPS (cm) | `0.001f` |
| Tài chính | Không dùng float |

`1e-6f` quá nhỏ cho cảm biến nhiệt độ (noise lớn hơn epsilon → luôn "không bằng"). `0.01f` quá lớn cho tính toán chính xác (2 số thực sự khác nhau nhưng bị coi là bằng).

</details>

---

**A07.** `3.14` và `3.14f` khác nhau thế nào trong C? Tại sao trong embedded phải luôn thêm `f`?

<details>
<summary>Xem đáp án</summary>

- `3.14` → **`double`** (64-bit) — default của mọi float literal trong C
- `3.14f` → **`float`** (32-bit)

**Tại sao embedded phải luôn thêm `f`:**

```c
float x = 0.1f;
float y = x * 2.0;   // 2.0 là double → x convert lên double → tính double
float z = x * 2.0f;  // 2.0f là float → tính thẳng float
```

Khi dùng `double` literal:
- `float` bị implicit convert lên `double`
- Trên MCU không có double FPU → **software emulation** → chậm hơn 10-100 lần
- Tốn RAM hơn (8 bytes vs 4 bytes)

**Rule:** Mọi float literal trong embedded code đều phải có hậu tố `f`.

</details>

---

**A08.** `0.1f > 0.1` đúng hay sai? Giải thích tại sao.

<details>
<summary>Xem đáp án</summary>

**Đúng** — `0.1f > 0.1`.

- `0.1f` (32-bit): `0.100000001490116...` — sai số dương, **lớn hơn** 0.1 thật
- `0.1` (64-bit): `0.100000000000000005...` — gần 0.1 thật hơn

`0.1f` có ít bit mantissa hơn → round lên → kết quả lớn hơn `0.1` double.

**Lưu ý:** Không phải float **luôn** lớn hơn double — tùy giá trị, sai số có thể dương hoặc âm. `0.1f > 0.1` là đúng, nhưng `0.2f` có thể lớn hoặc nhỏ hơn `0.2` tùy cách round.

</details>

---

**A09.** Tại sao `scanf` cần `&x` còn `printf` chỉ cần `x`?

<details>
<summary>Xem đáp án</summary>

- `printf` → **đọc** giá trị biến → chỉ cần giá trị `x` (pass by value)
- `scanf` → **ghi** giá trị vào biến → cần **địa chỉ** `&x` để biết ghi vào đâu (pass by pointer)

Nếu `scanf` chỉ nhận `x` (giá trị), nó không thể thay đổi biến `x` gốc vì C pass by value — sửa bản copy, không sửa bản gốc.

```c
int x = 0;
scanf("%d", x);   // sai! scanf nhận giá trị 0, không biết địa chỉ của x
scanf("%d", &x);  // đúng! scanf nhận địa chỉ, ghi thẳng vào x
```

</details>

---

**A10.** `scanf("%s")` dừng khi nào? Tại sao đây là nguồn gốc của buffer overflow?

<details>
<summary>Xem đáp án</summary>

`scanf("%s")` dừng khi gặp **whitespace** (space, tab, newline) — không phải chỉ Enter.

**Buffer overflow:**
```c
char name[8];
scanf("%s", name);  // không giới hạn độ dài!
```

Nếu input dài hơn 7 ký tự, `scanf` tiếp tục ghi **vượt quá boundary** của `name[8]`:
```
[N][g][u][y][e][n][n][n] ← hết buffer
[n][n][\0]               ← ghi vào vùng nhớ của biến khác / return address
```

Hậu quả: data corruption, crash, security vulnerability (attacker kiểm soát return address → arbitrary code execution).

**Fix:** `scanf("%7s", name)` hoặc dùng `fgets`.

</details>

---

**A11.** `fgets` khác `scanf("%s")` những điểm nào? Khi nào dùng cái nào?

<details>
<summary>Xem đáp án</summary>

| | `scanf("%s")` | `fgets` |
|---|---|---|
| Khoảng trắng | Dừng lại | Đọc được |
| Giới hạn độ dài | Cần thêm `%7s` | Tự động qua `sizeof` |
| `\n` trong buffer | Không lưu | Lưu vào buffer |
| An toàn | Nguy hiểm | An toàn hơn |

**Khi nào dùng:**
- `scanf("%s")`: học C trên PC, input đơn giản không có space, chỉ đọc 1 token
- `fgets`: input có space (họ tên, câu lệnh), đọc từng dòng, embedded UART command, đọc file config

**Thực tế embedded production code:** gần như không dùng `scanf` — dùng `fgets` hoặc parse binary protocol.

</details>

---

**A12.** `fgets` lưu `\n` vào buffer — tại sao điều này gây bug? Cách fix chuẩn là gì?

<details>
<summary>Xem đáp án</summary>

```c
char name[32];
fgets(name, sizeof(name), stdin);
// Input: "Giang" + Enter
// name = "Giang\n"  ← có '\n'!

if (strcmp(name, "Giang") == 0)  // FALSE! "Giang\n" ≠ "Giang"
```

**Fix chuẩn:**
```c
name[strcspn(name, "\n")] = '\0';
```

`strcspn(name, "\n")` trả về index của `\n` đầu tiên. Gán `'\0'` vào → cắt bỏ `\n`.

**Tại sao không dùng `name[strlen(name)-1] = '\0'`?**

Nếu input đủ dài để fill buffer, `fgets` không lưu `\n` (vì hết chỗ) → `strlen(name)-1` trỏ vào ký tự cuối hợp lệ → xóa nhầm. `strcspn` an toàn hơn vì chỉ xóa nếu tìm thấy `\n`.

</details>

---

**A13.** 3 stream `stdin`/`stdout`/`stderr` là gì? OS tạo chúng khi nào?

<details>
<summary>Xem đáp án</summary>

3 stream mặc định OS tự động mở khi chương trình khởi động:

| Stream | File descriptor | Hướng | Mặc định nối với |
|---|---|---|---|
| `stdin` | 0 | Input | Keyboard |
| `stdout` | 1 | Output | Terminal |
| `stderr` | 2 | Error output | Terminal |

OS tạo chúng **trước khi `main()` chạy** — cụ thể là khi `_start()` (C runtime entry point) khởi tạo process.

Trong C, chúng là `FILE*` pointer được khai báo trong `<stdio.h>`:
```c
extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;
```

</details>

---

**A14.** `stdout` và `stderr` đều in ra terminal — tại sao cần 2 stream riêng thay vì dùng `stdout` cho tất cả?

<details>
<summary>Xem đáp án</summary>

**3 lý do chính:**

1. **Redirect độc lập:**
```bash
./program > log.txt        # stdout vào file, stderr vẫn ra terminal
./program 2> errors.txt    # stderr vào file, stdout ra terminal
```

2. **Buffer khác nhau:**
   - `stdout` line buffered → có thể mất nếu crash trước khi flush
   - `stderr` unbuffered → in ngay lập tức → không mất dù crash

3. **Phân tách data và error:**
   - `stdout`: data bình thường, có thể pipe sang tool khác
   - `stderr`: thông báo lỗi, luôn cần người đọc ngay

**Ứng dụng thực tế:**
```bash
./sensor_program > sensor_data.csv 2> errors.log
# Sau 8 tiếng: sensor_data.csv có hàng nghìn dòng
# errors.log chỉ có lỗi → dễ tìm ngay vấn đề
```

</details>

---

**A15.** `stdout` unbuffered hay line buffered? `stderr` thì sao? Tại sao thiết kế vậy?

<details>
<summary>Xem đáp án</summary>

- `stdout`: **line buffered** khi nối với terminal (full buffered khi redirect vào file)
- `stderr`: **unbuffered** — ghi ra ngay lập tức

**Tại sao `stderr` unbuffered:**

Lỗi cần hiện ra ngay — nếu program crash sau khi ghi lỗi:
```c
fprintf(stderr, "Critical error!\n");  // in ngay → thấy được
*null_ptr = 1;                         // crash
```

Nếu `stderr` có buffer:
```c
printf("Critical error!\n");  // vào buffer stdout
*null_ptr = 1;                 // crash → buffer chưa flush → MẤT thông báo!
```

**Tại sao `stdout` line buffered:**
- Hiệu suất — gom nhiều `printf` nhỏ thành 1 lần write syscall
- Vẫn hợp lý — mỗi `\n` flush → người dùng thấy từng dòng hoàn chỉnh

</details>

---

**A16.** Tại sao không nên dùng `float` cho tính toán tài chính? Thay thế bằng gì?

<details>
<summary>Xem đáp án</summary>

Float có sai số tích lũy — không thể biểu diễn nhiều giá trị tiền tệ chính xác:

```c
float price = 0.1f + 0.2f;
printf("%.10f\n", price);  // 0.3000000119 — không phải 0.30!
```

Trong tài chính, sai số dù nhỏ cũng không chấp nhận được — `0.30` phải là `0.30` chính xác.

**Thay thế — dùng integer với đơn vị nhỏ nhất:**
```c
// Lưu số tiền bằng cent (×100)
int price_cents = 10 + 20;  // 30 cent = $0.30 chính xác
printf("$%d.%02d\n", price_cents/100, price_cents%100);  // $0.30
```

Hoặc dùng `double` với `round()` khi hiển thị — nhưng integer vẫn là cách chuẩn nhất.

</details>

---

**A17.** `fabsf()` khác `fabs()` và `abs()` thế nào? Dùng sai có hậu quả gì?

<details>
<summary>Xem đáp án</summary>

| Hàm | Kiểu | Header |
|---|---|---|
| `abs(x)` | `int` | `<stdlib.h>` |
| `fabs(x)` | `double` | `<math.h>` |
| `fabsf(x)` | `float` | `<math.h>` |
| `fabsl(x)` | `long double` | `<math.h>` |

**Hậu quả dùng sai:**

```c
float x = -3.14f;
float r = abs(x);    // abs() cho int → truncate -3.14 → -3 → abs(-3) = 3
                     // nhưng mất phần thập phân!

float r2 = fabs(x);  // đúng về mặt math nhưng x convert lên double
                     // trên MCU không có double FPU → software emulation → chậm

float r3 = fabsf(x); // đúng và nhanh nhất cho float
```

**Rule trong embedded:** `fabsf()` cho `float`, `fabs()` cho `double`.

</details>

---

**A18.** Khi nào nên dùng `%e` thay vì `%f` để in số thực?

<details>
<summary>Xem đáp án</summary>

`%e` in dạng scientific notation: `1.234000e+05`

**Dùng `%e` khi:**
- Giá trị rất lớn hoặc rất nhỏ mà `%f` in ra nhiều số 0 vô nghĩa
- Debug giá trị epsilon, sai số tính toán
- In kết quả khoa học/kỹ thuật cần thấy rõ số mũ

```c
float big   = 1234567.0f;
float small = 0.000001234f;

printf("%f\n", big);    // 1234567.000000     ← OK
printf("%f\n", small);  // 0.000001           ← mất precision
printf("%e\n", small);  // 1.234000e-06       ← rõ hơn
printf("%e\n", 1e-6f);  // 1.000000e-06
```

**Trong embedded sensor:** dùng `%e` khi in giá trị ADC sau calibration có nhiều số thập phân.

</details>

---

**A19.** Redirect `2>&1` có nghĩa là gì? Khi nào dùng?

<details>
<summary>Xem đáp án</summary>

`2>&1` = redirect **stderr (fd 2) vào cùng chỗ với stdout (fd 1)**

```bash
./program > output.txt 2>&1
# stdout → output.txt
# stderr → cũng vào output.txt (vì redirect vào stdout)
# → cả 2 stream vào cùng 1 file, theo thứ tự thực tế
```

**Khi dùng:**
```bash
# Ghi tất cả output (cả lỗi) vào 1 file log để xem sau
./long_running_program > full_log.txt 2>&1

# Bỏ hoàn toàn mọi output (cả error)
./quiet_program > /dev/null 2>&1

# Pipe cả stdout và stderr vào grep
./program 2>&1 | grep "ERROR"
```

**Chú ý thứ tự:** `> file 2>&1` (đúng) vs `2>&1 > file` (sai — stderr redirect vào stdout cũ trước khi stdout redirect vào file).

</details>

---

**A20.** Trong embedded, tại sao `scanf` gần như không được dùng trong production code?

<details>
<summary>Xem đáp án</summary>

**4 lý do chính:**

1. **Không kiểm soát buffer overflow** — `scanf("%s")` không giới hạn độ dài mặc định
2. **Khó handle error** — nếu người dùng nhập sai kiểu (`"abc"` cho `%d`), `scanf` để lại data trong buffer, gây loop vô tận
3. **Không có keyboard trên MCU** — data đến qua UART, SPI, I2C dưới dạng binary hoặc structured protocol, không phải interactive input
4. **Blocking call** — `scanf` block đến khi nhận đủ input, không phù hợp với real-time embedded system

**Thay thế trong embedded:**
```c
// Nhận command qua UART
char buf[64];
fgets(buf, sizeof(buf), uart_stream);

// Parse binary protocol
uint8_t packet[8];
read_uart(packet, 8);
uint16_t value = (packet[3] << 8) | packet[4];
```

</details>

---

## Phần B — Đọc Code (Code Reading)

---

**B01.** Output là gì? Giải thích tại sao.

```c
#include <stdio.h>

int main(void) {
    unsigned int x = 4294967295u;  // 0xFFFFFFFF
    printf("%d\n", x);
    printf("%u\n", x);
    printf("%x\n", x);
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

```
-1
4294967295
ffffffff
```

- `%d`: đọc `0xFFFFFFFF` như signed 32-bit → two's complement → `-1`
- `%u`: đọc như unsigned 32-bit → `4294967295`
- `%x`: in hex → `ffffffff`

Cùng bit pattern, 3 cách đọc khác nhau — `printf` không quan tâm kiểu biến.

</details>

---

**B02.** Output là gì?

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint8_t  a = 0xAB;
    uint16_t b = 0x1234;
    uint32_t c = 0x00000005;

    printf("%02X\n",  a);
    printf("%04X\n",  b);
    printf("%08X\n",  c);
    printf("%2X\n",   a);
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

```
AB
1234
00000005
AB
```

- `%02X`: min 2 ký tự, pad `0` → `AB` (đã đủ 2)
- `%04X`: min 4 ký tự → `1234` (đã đủ 4)
- `%08X`: min 8 ký tự, pad `0` → `00000005`
- `%2X`: min 2 ký tự, pad **space** (không có `0`) → `AB` (đã đủ, không pad)

Phân biệt `%02X` (pad zero) và `%2X` (pad space).

</details>

---

**B03.** Đoạn code này có bug không? Output là gì?

```c
#include <stdio.h>
#include <math.h>

int main(void) {
    float a = 0.1f + 0.2f;
    float b = 0.3f;

    if (a == b) {
        printf("bang nhau\n");
    } else {
        printf("khac nhau\n");
    }

    printf("a = %.10f\n", a);
    printf("b = %.10f\n", b);
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

**Có bug — không nên dùng `==` với float.**

Output thực tế **phụ thuộc nền tảng:**
- x86 với FPU 80-bit: có thể in `"bang nhau"`
- ARM MCU FPU 32-bit: thường in `"khac nhau"`

```
// Trên ARM MCU:
khac nhau
a = 0.3000000119
b = 0.3000000119   ← trông giống nhau nhưng bit pattern khác!
```

**Fix:**
```c
#define EPSILON 1e-6f
if (fabsf(a - b) < EPSILON) { printf("bang nhau\n"); }
```

</details>

---

**B04.** Output là gì? Có vấn đề gì không?

```c
#include <stdio.h>

int main(void) {
    char name[6];
    scanf("%s", name);  // input: "Nguyen"
    printf("Hello %s\n", name);
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

Input `"Nguyen"` = 6 ký tự + `'\0'` = 7 bytes — nhưng `name[6]` chỉ chứa 6 bytes!

**Buffer overflow** — `'\0'` ghi vào `name[6]` (ngoài array) → **Undefined Behavior**.

May mắn thì in `"Hello Nguyen"`, nhưng đây là UB — có thể crash, data corruption.

**Fix:**
```c
char name[7];   // 6 ký tự + '\0'
scanf("%6s", name);  // giới hạn 6 ký tự
// hoặc
fgets(name, sizeof(name), stdin);
```

</details>

---

**B05.** Output là gì?

```c
#include <stdio.h>

int main(void) {
    char buf[32];
    fgets(buf, sizeof(buf), stdin);  // input: "Hello World"
    printf("[%s]\n", buf);
    printf("length = %zu\n", strlen(buf));
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

```
[Hello World
]
length = 12
```

`fgets` lưu `\n` vào buffer → `buf = "Hello World\n"` → 11 ký tự + `\n` + `\0` = 13 bytes, `strlen = 12`.

Output có xuống dòng giữa `]` — `printf` in `\n` từ buffer, rồi `\n` từ format string tạo thêm 1 dòng trống.

**Fix nếu không muốn `\n`:**
```c
buf[strcspn(buf, "\n")] = '\0';
```

</details>

---

**B06.** Đoạn code này print gì? Tại sao kết quả "vô lý"?

```c
#include <stdio.h>
#include <math.h>

int main(void) {
    float x = 1234567.0f;
    float y = x + 1.0f;

    if (x == y) {
        printf("bang nhau\n");
    } else {
        printf("khac nhau\n");
    }
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

**Output:** `"bang nhau"` — dù `1234567 + 1 = 1234568` rõ ràng khác!

**Lý do — float precision giới hạn:**

`float` chỉ có ~7 chữ số thập phân có nghĩa. `1234567.0f` đã dùng hết 7 chữ số — không còn precision để biểu diễn `+1`:

```
1234567.0f = 1234567.0  (7 chữ số)
1234568.0f = 1234568.0  (7 chữ số)

Nhưng: 1234567.0f + 1.0f = 1234567.0f (1.0 quá nhỏ so với precision)
```

Đây là lý do tại sao float **không dùng được cho counter lớn hoặc tài chính**.

</details>

---

**B07.** Output là gì?

```c
#include <stdio.h>

int main(void) {
    int x = 255;
    printf("%d\n",   x);
    printf("%05d\n", x);
    printf("%-5d|\n", x);
    printf("%+d\n",  x);
    printf("%+d\n",  -x);
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

```
255
00255
255  |
+255
-255
```

- `%d`: bình thường → `255`
- `%05d`: min 5 ký tự, pad `0` → `00255`
- `%-5d|`: căn **trái**, pad space → `255  |` (2 space sau)
- `%+d`: luôn in dấu → `+255`
- `%+d` với số âm: → `-255`

</details>

---

**B08.** Tìm bug trong đoạn code này:

```c
#include <stdio.h>

int main(void) {
    float celsius = 36.6f;
    float fahrenheit = celsius * 9 / 5 + 32;
    printf("%.1f C = %.1f F\n", celsius, fahrenheit);
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

**Không có bug — nhưng có điểm cần chú ý.**

`9 / 5` là **integer division** = `1` (không phải `1.8`). Nhưng C tính từ trái sang phải:

```
celsius * 9 / 5 + 32
= (celsius * 9) / 5 + 32
= (36.6f * 9) / 5 + 32   ← float * int → float (329.4f)
= 329.4f / 5 + 32        ← float / int → float (65.88f)
= 65.88f + 32            ← float + int → float (97.88f)
```

Kết quả đúng: `36.6 C = 97.9 F` ✅

**Nhưng nếu viết:** `celsius * (9/5) + 32` → `9/5 = 1` (integer division) → kết quả sai `68.6 F`.

**Best practice — rõ ràng hơn:**
```c
float fahrenheit = celsius * 9.0f / 5.0f + 32.0f;
```

</details>

---

**B09.** Output là gì?

```c
#include <stdio.h>
#include <math.h>

int main(void) {
    float a = 0.1f;
    float b = 0.1f;
    float c = 0.1f;
    float sum = a + b + c;

    printf("%.10f\n", sum);
    printf("%d\n", sum == 0.3f);
    printf("%d\n", fabsf(sum - 0.3f) < 1e-6f);
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

```
0.3000000119
0
1
```

- `sum = 0.1f + 0.1f + 0.1f = 0.3000000119...` (sai số tích lũy)
- `sum == 0.3f` → `0` (false) vì bit pattern khác nhau
- `fabsf(sum - 0.3f) < 1e-6f` → `|0.3000000119 - 0.3000000119| < 1e-6` → `0 < 1e-6` → `1` (true)

Đây là cách so sánh float đúng.

</details>

---

**B10.** Đoạn code này có vấn đề gì?

```c
#include <stdio.h>

int main(void) {
    char buf[10];
    printf("Nhap ten: ");
    scanf("%s", buf);

    char confirm[4];
    printf("Xac nhan (y/n): ");
    scanf("%s", confirm);

    printf("Ten: %s, Xac nhan: %s\n", buf, confirm);
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

**2 vấn đề:**

1. **Buffer overflow trên `buf[10]`** — nếu nhập tên > 9 ký tự → tràn stack, ghi đè `confirm` hoặc return address.

2. **Buffer overflow trên `confirm[4]`** — nếu nhập > 3 ký tự → tràn.

3. **`scanf` không đọc được tên có dấu cách** — dừng ở space đầu tiên.

**Fix:**
```c
char buf[32];
fgets(buf, sizeof(buf), stdin);
buf[strcspn(buf, "\n")] = '\0';

char confirm[8];
fgets(confirm, sizeof(confirm), stdin);
confirm[strcspn(confirm, "\n")] = '\0';
```

</details>

---

**B11.** Output là gì? Giải thích `%.2f` và `%10.2f`.

```c
#include <stdio.h>

int main(void) {
    float pi = 3.14159265f;
    printf("%.2f\n",   pi);
    printf("%10.2f\n", pi);
    printf("%-10.2f|\n", pi);
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

```
3.14
      3.14|
3.14      |
```

- `%.2f`: 2 chữ số thập phân → `3.14`
- `%10.2f`: min 10 ký tự tổng, 2 thập phân, căn phải → `      3.14` (6 space trước)
- `%-10.2f|`: min 10 ký tự, căn **trái** → `3.14      |` (6 space sau)

**Format đầy đủ:** `%[flag][width][.precision]specifier`

</details>

---

**B12.** Đoạn code này in gì ra stderr và stdout?

```c
#include <stdio.h>

int main(void) {
    printf("Starting program\n");
    fprintf(stderr, "Warning: low memory\n");
    printf("Processing data\n");
    fprintf(stderr, "Error: sensor timeout\n");
    printf("Done\n");
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

**stdout:**
```
Starting program
Processing data
Done
```

**stderr:**
```
Warning: low memory
Error: sensor timeout
```

Khi chạy bình thường trên terminal: cả 2 stream hiện lẫn lộn theo thứ tự thực tế (vì đều ra terminal).

Khi redirect:
```bash
./program > out.txt 2> err.txt
# out.txt: Starting program / Processing data / Done
# err.txt: Warning: low memory / Error: sensor timeout
```

</details>

---

**B13.** Tìm bug:

```c
#include <stdio.h>
#include <math.h>

int safe_divide(float a, float b, float *result) {
    if (b == 0.0f) {
        fprintf(stderr, "Error: divide by zero\n");
        return -1;
    }
    *result = a / b;
    return 0;
}

int main(void) {
    float r;
    float x = 1.0f - 0.9f - 0.1f;  // tưởng = 0.0f
    safe_divide(10.0f, x, &r);
    printf("%.2f\n", r);
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

**Bug — `b == 0.0f` không hoạt động đúng với float.**

`1.0f - 0.9f - 0.1f` không bằng chính xác `0.0f` do sai số float:
```
1.0f - 0.9f = 0.100000024...
0.100000024 - 0.1f = -1.49e-8  (≠ 0)
```

→ `b == 0.0f` là false → không check được → `10.0f / (-1.49e-8)` = số rất lớn.

**Fix:**
```c
if (fabsf(b) < 1e-7f) {  // kiểm tra gần bằng 0
    fprintf(stderr, "Error: divide by zero\n");
    return -1;
}
```

</details>

---

**B14.** Output là gì?

```c
#include <stdio.h>

int main(void) {
    double x = 3.14;
    float  y = 3.14f;

    printf("double: %.15f\n", x);
    printf("float:  %.15f\n", y);
    printf("equal:  %d\n", x == y);
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

```
double: 3.141592653589793
float:  3.140000104904175
equal:  0
```

- `double 3.14`: 15-16 chữ số có nghĩa → `3.141592653589793`
- `float 3.14f`: ~7 chữ số có nghĩa → `3.1400001...` (sai số lớn hơn)
- `x == y`: `float y` convert lên `double` → `3.14000010490...` ≠ `3.14159265...` → false

Đây là lý do không mix `float` và `double` trong cùng biểu thức.

</details>

---

**B15.** Đoạn code này làm gì? Có bug không?

```c
#include <stdio.h>
#include <string.h>

int main(void) {
    char line[64];
    while (fgets(line, sizeof(line), stdin) != NULL) {
        line[strcspn(line, "\n")] = '\0';
        if (strlen(line) == 0) break;
        printf("Got: [%s]\n", line);
    }
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

**Không bug** — đây là pattern chuẩn đọc nhiều dòng từ stdin.

**Flow:**
1. `fgets` đọc từng dòng — trả về `NULL` khi EOF
2. `strcspn` xóa `\n`
3. `strlen == 0` → dừng khi gặp dòng trống (Enter)
4. In từng dòng trong dấu `[]`

**Ví dụ input/output:**
```
Input:  "Hello"  → Got: [Hello]
Input:  "World"  → Got: [World]
Input:  ""       → break (dòng trống)
```

`fgets != NULL` check quan trọng — tránh crash khi EOF hoặc pipe đóng.

</details>

---

**B16.** Output là gì? Giải thích tại sao `%p` in ra giá trị đó.

```c
#include <stdio.h>

int main(void) {
    int x = 42;
    int *p = &x;
    printf("x   = %d\n",  x);
    printf("&x  = %p\n",  (void*)&x);
    printf("p   = %p\n",  (void*)p);
    printf("*p  = %d\n",  *p);
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

```
x   = 42
&x  = 0x7ffd1234abcd   (địa chỉ thực tế thay đổi mỗi lần chạy)
p   = 0x7ffd1234abcd   (cùng địa chỉ với &x)
*p  = 42
```

- `%p` in địa chỉ dạng hex — cần cast về `(void*)` để đúng chuẩn
- `&x` và `p` cùng giá trị vì `p = &x`
- `*p` dereference → lấy giá trị tại địa chỉ p → `42`

Địa chỉ thay đổi mỗi lần chạy do **ASLR** (Address Space Layout Randomization).

</details>

---

**B17.** Tìm tất cả vấn đề:

```c
#include <stdio.h>

int main(void) {
    float temp = 98.6;           // (1)
    double threshold = 37.0f;    // (2)
    int count = 1000000;

    if (temp > threshold) {      // (3)
        printf("Fever detected: %f\n", temp);
    }

    printf("Count: %d\n", count);  // (4)
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

**(1) `float temp = 98.6`** — `98.6` là `double` literal, gán vào `float` → implicit conversion, mất precision. Fix: `98.6f`.

**(2) `double threshold = 37.0f`** — `37.0f` là `float` literal, gán vào `double` → convert lên, không mất data nhưng không cần thiết. Fix: `37.0` hoặc giữ `double threshold = 37.0f` cũng được.

**(3) `temp > threshold`** — `float` vs `double` → `temp` convert lên `double`. Kết quả đúng về logic nhưng tốn thêm conversion. Fix: dùng cùng kiểu.

**(4) `printf("Count: %d\n", count)`** — `int` với `%d` là đúng trên 32-bit. Nhưng nếu `count` là `long` hoặc trên hệ thống `int` = 16-bit thì sai. Best practice dùng `%d` với `int32_t` và `%ld` với `long`.

</details>

---

**B18.** Output là gì?

```c
#include <stdio.h>

int main(void) {
    printf("%d\n",  sizeof(float));
    printf("%d\n",  sizeof(double));
    printf("%d\n",  sizeof(char));
    printf("%zu\n", sizeof(float));   // đúng cách
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

```
4        (hoặc warning vì %d với size_t)
8        (hoặc warning)
1        (hoặc warning)
4
```

- `sizeof(float)` = 4 bytes
- `sizeof(double)` = 8 bytes
- `sizeof(char)` = 1 byte (luôn luôn)

**Vấn đề:** 3 dòng đầu dùng `%d` cho `size_t` — UB trên 64-bit (size_t = 8 bytes, %d đọc 4 bytes). Compiler warn: `format '%d' expects 'int' but argument is 'long unsigned int'`.

Dòng cuối dùng `%zu` → đúng.

</details>

---

**B19.** Đoạn code này làm gì khi chạy trên embedded UART?

```c
#include <stdio.h>

void log_sensor(float temperature, uint16_t adc_val) {
    printf("TEMP=%.2f,ADC=0x%04X\n", temperature, adc_val);
    fflush(stdout);
}
```

<details>
<summary>Xem đáp án</summary>

In dữ liệu sensor ra UART theo format CSV-like:
```
TEMP=36.60,ADC=0x0ABC
```

**Điểm đáng chú ý:**
- `%.2f`: nhiệt độ 2 chữ số thập phân
- `%04X`: ADC value hex, luôn 4 nibble (16-bit)
- `fflush(stdout)`: **quan trọng trong embedded** — đảm bảo data gửi qua UART ngay lập tức, không đợi buffer đầy

Nếu không có `fflush`: dữ liệu có thể delay vài giây trong buffer → debugging khó khăn.

</details>

---

**B20.** Output là gì? Có UB không?

```c
#include <stdio.h>

int main(void) {
    char buf[4];
    fgets(buf, sizeof(buf), stdin);  // input: "Hello"
    printf("[%s]\n", buf);
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

**Không UB** — `fgets` tự giới hạn `sizeof(buf) - 1 = 3` ký tự + `'\0'`.

Input `"Hello"` → `fgets` chỉ đọc `"Hel"` + `'\0'` vào `buf[4]`:
```
buf = ['H', 'e', 'l', '\0']
```

Output: `[Hel]`

"lo\n" còn lại trong stdin buffer — lần `fgets` tiếp theo sẽ đọc tiếp.

Đây là lý do `fgets` **an toàn hơn** `scanf("%s")` — tự động không vượt quá buffer.

</details>

---

**B21.** Đoạn code sau print gì? Giải thích `%e`.

```c
#include <stdio.h>

int main(void) {
    float big   = 123456789.0f;
    float small = 0.000000123f;
    float normal = 3.14f;

    printf("%f\n",  big);
    printf("%e\n",  big);
    printf("%f\n",  small);
    printf("%e\n",  small);
    printf("%g\n",  normal);
    printf("%g\n",  big);
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

```
123456792.000000    (sai số float — không phải 123456789!)
1.234568e+08
0.000000            (quá nhỏ, %f mất precision)
1.230000e-07
3.14                (%g chọn f hoặc e tùy giá trị)
1.23457e+08         (%g dùng e vì số quá lớn)
```

- `%e`: scientific notation — luôn dùng cho số rất lớn/nhỏ
- `%g`: tự động chọn `%f` hoặc `%e` tùy giá trị (hữu ích khi không biết trước magnitude)
- `big = 123456792.0f`: float chỉ có ~7 chữ số → `123456789` bị round thành `123456792`

</details>

---

**B22.** Tìm bug trong embedded logger này:

```c
#include <stdio.h>

void log_error(const char *msg) {
    printf("[ERROR] %s\n", msg);
}

void log_info(const char *msg) {
    printf("[INFO] %s\n", msg);
}

int sensor_init(void) {
    log_info("Initializing sensor");
    if (sensor_connect() < 0) {
        log_error("Connection failed");
        return -1;
    }
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

**Bug:** Cả 2 hàm đều dùng `stdout` — không phân biệt error và info khi redirect.

```bash
./program > info.log
# info.log có cả INFO lẫn ERROR!
```

**Fix:**
```c
void log_error(const char *msg) {
    fprintf(stderr, "[ERROR] %s\n", msg);  // stderr → unbuffered, redirectable
}

void log_info(const char *msg) {
    printf("[INFO] %s\n", msg);             // stdout
}
```

Thêm `fflush` nếu cần đảm bảo output ngay lập tức:
```c
void log_error(const char *msg) {
    fprintf(stderr, "[ERROR] %s\n", msg);
    // stderr đã unbuffered — không cần fflush
}
```

</details>

---

**B23.** Output là gì?

```c
#include <stdio.h>
#include <math.h>

int main(void) {
    float x = -3.7f;
    printf("%.1f\n", fabsf(x));
    printf("%.1f\n", ceilf(x));
    printf("%.1f\n", floorf(x));
    printf("%.1f\n", roundf(x));
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

```
3.7
-3.0
-4.0
-4.0
```

- `fabsf(-3.7)` = `3.7` (giá trị tuyệt đối)
- `ceilf(-3.7)` = `-3.0` (làm tròn **lên** — về phía +∞)
- `floorf(-3.7)` = `-4.0` (làm tròn **xuống** — về phía -∞)
- `roundf(-3.7)` = `-4.0` (làm tròn theo toán học — 0.7 > 0.5 nên ra -4)

**Chú ý:** `ceil` và `floor` theo chiều của trục số thực, không phải "lên/xuống" theo giá trị tuyệt đối.

</details>

---

**B24.** Đoạn code có vấn đề gì?

```c
#include <stdio.h>

int read_command(char *buf, int size) {
    printf("Enter command: ");
    return fgets(buf, size, stdin) != NULL;
}

int main(void) {
    char cmd[16];
    while (read_command(cmd, sizeof(cmd))) {
        cmd[strcspn(cmd, "\n")] = '\0';
        if (strcmp(cmd, "quit") == 0) break;
        printf("Command: %s\n", cmd);
    }
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

**Vấn đề nhỏ — thiếu `fflush` sau prompt:**

```c
printf("Enter command: ");
// stdout là line buffered — không có '\n' → prompt có thể không hiện ngay!
```

**Fix:**
```c
printf("Enter command: ");
fflush(stdout);  // flush ngay để user thấy prompt trước khi gõ
```

**Hoặc dùng `\n` sau prompt** (nhưng xấu về UI):
```c
printf("Enter command:\n");
```

Phần còn lại của code đúng — `fgets` an toàn, `strcspn` xóa `\n`, check `quit` hợp lý.

</details>

---

**B25.** Output là gì? Giải thích `fflush` ảnh hưởng thế nào.

```c
#include <stdio.h>
#include <unistd.h>

int main(void) {
    printf("Step 1");
    sleep(2);
    printf(" Step 2");
    fflush(stdout);
    sleep(1);
    printf(" Step 3\n");
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

**Timeline:**
- t=0: `printf("Step 1")` → vào buffer, **chưa hiện**
- t=0→2: sleep 2 giây — màn hình trống
- t=2: `printf(" Step 2")` → vào buffer, vẫn chưa hiện
- t=2: `fflush(stdout)` → flush buffer → **"Step 1 Step 2" hiện ngay**
- t=2→3: sleep 1 giây
- t=3: `printf(" Step 3\n")` → `\n` trigger flush → **" Step 3" hiện**

**Output cuối cùng:**
```
Step 1 Step 2 Step 3
```

Nhưng **thời điểm hiện** khác nhau:
- `"Step 1 Step 2"` hiện sau 2 giây (do `fflush`)
- `" Step 3"` hiện sau 3 giây (do `\n`)

</details>

---

## Phần C — Viết Code (Coding)

---

**C01.** Viết hàm `print_hex_dump(uint8_t *data, size_t len)` in dữ liệu dạng hex dump như debugger — mỗi dòng 16 bytes, kèm địa chỉ offset và ASCII representation.

```
Output mẫu:
0000: 48 65 6C 6C 6F 20 57 6F  72 6C 64 21 0A 00 00 00  Hello Wo rld!...
```

<details>
<summary>Xem đáp án</summary>

```c
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

void print_hex_dump(uint8_t *data, size_t len) {
    for (size_t i = 0; i < len; i += 16) {
        // In offset
        printf("%04zX: ", i);

        // In hex bytes
        for (size_t j = 0; j < 16; j++) {
            if (i + j < len) {
                printf("%02X ", data[i + j]);
            } else {
                printf("   ");  // padding nếu dòng cuối không đủ 16
            }
            if (j == 7) printf(" ");  // khoảng trắng giữa 8 byte
        }

        printf(" ");

        // In ASCII
        for (size_t j = 0; j < 16 && i + j < len; j++) {
            uint8_t c = data[i + j];
            printf("%c", isprint(c) ? c : '.');
        }

        printf("\n");
    }
}
```

</details>

---

**C02.** Viết hàm `safe_float_equal(float a, float b, float epsilon)` so sánh 2 số float an toàn. Xử lý thêm trường hợp đặc biệt: cả 2 đều là `NaN` hoặc `Inf`.

<details>
<summary>Xem đáp án</summary>

```c
#include <math.h>
#include <stdint.h>

int safe_float_equal(float a, float b, float epsilon) {
    // NaN không bằng bất cứ thứ gì, kể cả chính nó
    if (isnan(a) || isnan(b)) return 0;

    // Inf: chỉ bằng nhau nếu cùng dấu
    if (isinf(a) || isinf(b)) return a == b;

    // So sánh bình thường với epsilon
    return fabsf(a - b) < epsilon;
}
```

**Test:**
```c
safe_float_equal(0.1f+0.2f, 0.3f, 1e-6f);  // 1 (true)
safe_float_equal(1.0f/0.0f, 1.0f/0.0f, 1e-6f);  // 1 (inf == inf)
safe_float_equal(0.0f/0.0f, 0.0f, 1e-6f);  // 0 (NaN)
```

</details>

---

**C03.** Viết hàm `celsius_to_fahrenheit` và `fahrenheit_to_celsius` sử dụng đúng float literal. Thêm kiểm tra range hợp lệ cho nhiệt độ cơ thể người (-50°C đến 100°C).

<details>
<summary>Xem đáp án</summary>

```c
#include <stdio.h>
#include <math.h>

#define TEMP_MIN_C  -50.0f
#define TEMP_MAX_C   100.0f

int celsius_to_fahrenheit(float celsius, float *fahrenheit) {
    if (celsius < TEMP_MIN_C || celsius > TEMP_MAX_C) {
        fprintf(stderr, "Error: temperature %.1f out of range\n", celsius);
        return -1;
    }
    *fahrenheit = celsius * 9.0f / 5.0f + 32.0f;
    return 0;
}

int fahrenheit_to_celsius(float fahrenheit, float *celsius) {
    float c = (fahrenheit - 32.0f) * 5.0f / 9.0f;
    if (c < TEMP_MIN_C || c > TEMP_MAX_C) {
        fprintf(stderr, "Error: temperature out of range\n");
        return -1;
    }
    *celsius = c;
    return 0;
}
```

</details>

---

**C04.** Viết chương trình đọc các số float từ stdin (mỗi số 1 dòng), tính trung bình, min, max. Dừng khi gặp dòng trống hoặc EOF. Dùng `fgets` để đọc input.

<details>
<summary>Xem đáp án</summary>

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

int main(void) {
    char buf[64];
    float sum = 0.0f;
    float min = FLT_MAX;
    float max = -FLT_MAX;
    int count = 0;

    while (fgets(buf, sizeof(buf), stdin) != NULL) {
        buf[strcspn(buf, "\n")] = '\0';
        if (strlen(buf) == 0) break;  // dòng trống → dừng

        float val = strtof(buf, NULL);
        sum += val;
        if (val < min) min = val;
        if (val > max) max = val;
        count++;
    }

    if (count == 0) {
        printf("No input\n");
        return 0;
    }

    printf("Count: %d\n",   count);
    printf("Sum:   %.2f\n", sum);
    printf("Avg:   %.2f\n", sum / count);
    printf("Min:   %.2f\n", min);
    printf("Max:   %.2f\n", max);
    return 0;
}
```

`strtof` an toàn hơn `atof` vì có thể phát hiện lỗi parse.

</details>

---

**C05.** Viết hàm `log_message(const char *level, const char *fmt, ...)` — một simple logger hỗ trợ variadic arguments như `printf`. `"ERROR"` và `"WARN"` ghi vào stderr, còn lại vào stdout.

<details>
<summary>Xem đáp án</summary>

```c
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

void log_message(const char *level, const char *fmt, ...) {
    FILE *stream = stdout;

    // ERROR và WARN → stderr
    if (strcmp(level, "ERROR") == 0 || strcmp(level, "WARN") == 0) {
        stream = stderr;
    }

    fprintf(stream, "[%s] ", level);

    va_list args;
    va_start(args, fmt);
    vfprintf(stream, fmt, args);
    va_end(args);

    fprintf(stream, "\n");
}

// Sử dụng:
int main(void) {
    log_message("INFO",  "System started, version %d.%d", 1, 0);
    log_message("WARN",  "Battery low: %.1f%%", 15.3f);
    log_message("ERROR", "Sensor %d timeout after %d ms", 2, 500);
    return 0;
}
```

`vfprintf` = `fprintf` nhưng nhận `va_list` thay vì `...` — chuẩn để wrap printf-like functions.

</details>

---

**C06.** Viết hàm `read_float_bounded(float min, float max, float *result)` đọc 1 số float từ stdin, kiểm tra trong khoảng `[min, max]`, yêu cầu nhập lại nếu sai. Dùng `fgets` + `strtof`.

<details>
<summary>Xem đáp án</summary>

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int read_float_bounded(float min, float max, float *result) {
    char buf[64];
    while (1) {
        printf("Enter value [%.1f - %.1f]: ", min, max);
        fflush(stdout);

        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            return -1;  // EOF
        }

        buf[strcspn(buf, "\n")] = '\0';

        char *end;
        float val = strtof(buf, &end);

        // Kiểm tra parse thành công và trong range
        if (end == buf || *end != '\0') {
            fprintf(stderr, "Invalid number, try again\n");
            continue;
        }

        if (val < min || val > max) {
            fprintf(stderr, "Out of range [%.1f - %.1f], try again\n", min, max);
            continue;
        }

        *result = val;
        return 0;
    }
}
```

</details>

---

**C07.** Viết chương trình in bảng chuyển đổi nhiệt độ Celsius sang Fahrenheit từ 0°C đến 100°C, bước 10°C. Format output căn cột đẹp.

```
Output mẫu:
Celsius  | Fahrenheit
---------|----------
   0.0   |    32.0
  10.0   |    50.0
  ...
```

<details>
<summary>Xem đáp án</summary>

```c
#include <stdio.h>

int main(void) {
    printf("%-10s| %s\n", "Celsius", "Fahrenheit");
    printf("---------|----------\n");

    for (int c = 0; c <= 100; c += 10) {
        float celsius    = (float)c;
        float fahrenheit = celsius * 9.0f / 5.0f + 32.0f;
        printf("%8.1f   | %8.1f\n", celsius, fahrenheit);
    }

    return 0;
}
```

**Output:**
```
Celsius   | Fahrenheit
----------|----------
     0.0  |     32.0
    10.0  |     50.0
    20.0  |     68.0
    30.0  |     86.0
    40.0  |    104.0
    50.0  |    122.0
    60.0  |    140.0
    70.0  |    158.0
    80.0  |    176.0
    90.0  |    194.0
   100.0  |    212.0
```

</details>

---

**C08.** Viết hàm `parse_csv_line(char *line, float *values, int max_values)` parse 1 dòng CSV chứa số float, trả về số lượng giá trị đọc được. Ví dụ: `"1.5,2.3,3.7"` → `{1.5, 2.3, 3.7}`, return `3`.

<details>
<summary>Xem đáp án</summary>

```c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int parse_csv_line(char *line, float *values, int max_values) {
    int count = 0;
    char *token = strtok(line, ",");

    while (token != NULL && count < max_values) {
        // Trim whitespace
        while (*token == ' ') token++;

        char *end;
        float val = strtof(token, &end);

        if (end != token) {  // parse thành công
            values[count++] = val;
        }

        token = strtok(NULL, ",");
    }

    return count;
}

// Test:
int main(void) {
    char line[] = "1.5, 2.3, 3.7, 4.0";
    float vals[10];
    int n = parse_csv_line(line, vals, 10);

    for (int i = 0; i < n; i++) {
        printf("vals[%d] = %.1f\n", i, vals[i]);
    }
    return 0;
}
```

</details>

---

**C09.** Viết chương trình UART command parser đơn giản cho embedded — đọc lệnh từ stdin, xử lý các lệnh: `led on`, `led off`, `temp`, `help`, `quit`.

<details>
<summary>Xem đáp án</summary>

```c
#include <stdio.h>
#include <string.h>

void cmd_led(const char *arg) {
    if (strcmp(arg, "on") == 0) {
        printf("[HW] LED ON\n");
    } else if (strcmp(arg, "off") == 0) {
        printf("[HW] LED OFF\n");
    } else {
        fprintf(stderr, "Usage: led on|off\n");
    }
}

void cmd_temp(void) {
    // Giả lập đọc sensor
    float temp = 36.5f;
    printf("Temperature: %.1f C\n", temp);
}

void cmd_help(void) {
    printf("Commands:\n");
    printf("  led on|off  - control LED\n");
    printf("  temp        - read temperature\n");
    printf("  help        - show this help\n");
    printf("  quit        - exit\n");
}

int main(void) {
    char buf[64];

    printf("Embedded CLI ready. Type 'help' for commands.\n");

    while (1) {
        printf("> ");
        fflush(stdout);

        if (fgets(buf, sizeof(buf), stdin) == NULL) break;
        buf[strcspn(buf, "\n")] = '\0';
        if (strlen(buf) == 0) continue;

        // Parse command và argument
        char cmd[32] = {0};
        char arg[32] = {0};
        sscanf(buf, "%31s %31s", cmd, arg);

        if (strcmp(cmd, "led")  == 0) cmd_led(arg);
        else if (strcmp(cmd, "temp") == 0) cmd_temp();
        else if (strcmp(cmd, "help") == 0) cmd_help();
        else if (strcmp(cmd, "quit") == 0) break;
        else fprintf(stderr, "Unknown command: %s\n", cmd);
    }

    printf("Bye.\n");
    return 0;
}
```

</details>

---

**C10.** Viết hàm `float_to_str(float val, char *buf, int buf_size, int decimal_places)` chuyển float thành string **không dùng `sprintf`** — tự implement bằng integer arithmetic. Hữu ích trên MCU không có `sprintf` floating point support.

<details>
<summary>Xem đáp án</summary>

```c
#include <stdint.h>
#include <string.h>
#include <math.h>

void float_to_str(float val, char *buf, int buf_size, int decimals) {
    if (buf_size < 2) return;

    int pos = 0;

    // Xử lý số âm
    if (val < 0.0f) {
        buf[pos++] = '-';
        val = -val;
    }

    // Làm tròn
    float multiplier = powf(10.0f, (float)decimals);
    int32_t scaled = (int32_t)(val * multiplier + 0.5f);

    int32_t int_part  = scaled / (int32_t)multiplier;
    int32_t frac_part = scaled % (int32_t)multiplier;

    // In phần nguyên
    char tmp[16];
    int len = 0;
    if (int_part == 0) {
        tmp[len++] = '0';
    } else {
        int32_t n = int_part;
        while (n > 0) {
            tmp[len++] = '0' + (n % 10);
            n /= 10;
        }
        // Đảo ngược
        for (int i = 0; i < len/2; i++) {
            char c = tmp[i]; tmp[i] = tmp[len-1-i]; tmp[len-1-i] = c;
        }
    }

    for (int i = 0; i < len && pos < buf_size-1; i++) {
        buf[pos++] = tmp[i];
    }

    // In phần thập phân
    if (decimals > 0 && pos < buf_size-1) {
        buf[pos++] = '.';
        // In từng chữ số thập phân
        int32_t mult = (int32_t)multiplier / 10;
        for (int i = 0; i < decimals && pos < buf_size-1; i++) {
            buf[pos++] = '0' + (frac_part / mult);
            frac_part %= mult;
            mult /= 10;
        }
    }

    buf[pos] = '\0';
}

// Test:
// float_to_str(36.75f, buf, 16, 2) → "36.75"
// float_to_str(-3.14f, buf, 16, 2) → "-3.14"
```

</details>

---

**C11.** Viết macro `PRINTF_CHECK` wrap lại `printf` để in thêm file name và line number — hữu ích khi debug embedded firmware.

```c
// Sử dụng:
PRINTF_CHECK("Sensor value: %d\n", adc_val);
// Output: [main.c:42] Sensor value: 1234
```

<details>
<summary>Xem đáp án</summary>

```c
#include <stdio.h>

#define PRINTF_CHECK(fmt, ...) \
    printf("[%s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)

// Hoặc version ghi ra stderr (tốt hơn cho debug):
#define DEBUG_PRINT(fmt, ...) \
    fprintf(stderr, "[%s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)

// Có thể enable/disable bằng compile flag:
#ifdef DEBUG
    #define DBG(fmt, ...) \
        fprintf(stderr, "[%s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
    #define DBG(fmt, ...)  // empty — không tốn code space trên MCU
#endif

// Test:
int main(void) {
    int val = 42;
    PRINTF_CHECK("Value = %d\n", val);
    // Output: [main.c:XX] Value = 42
    return 0;
}
```

`##__VA_ARGS__` — `##` xử lý trường hợp không có argument (tránh trailing comma lỗi).

</details>

---

**C12.** Viết chương trình đọc file config dạng `key=value` từ stdin và in ra từng cặp. Bỏ qua dòng bắt đầu bằng `#` (comment) và dòng trống.

```
# Config file
baudrate=115200
device=/dev/ttyS0
timeout=500
```

<details>
<summary>Xem đáp án</summary>

```c
#include <stdio.h>
#include <string.h>

void parse_config(void) {
    char line[128];
    int line_num = 0;

    while (fgets(line, sizeof(line), stdin) != NULL) {
        line_num++;
        line[strcspn(line, "\n")] = '\0';

        // Bỏ qua dòng trống
        if (strlen(line) == 0) continue;

        // Bỏ qua comment
        if (line[0] == '#') continue;

        // Tìm dấu '='
        char *eq = strchr(line, '=');
        if (eq == NULL) {
            fprintf(stderr, "Line %d: invalid format (missing '=')\n", line_num);
            continue;
        }

        // Tách key và value
        *eq = '\0';
        char *key   = line;
        char *value = eq + 1;

        printf("key=[%s] value=[%s]\n", key, value);
    }
}

int main(void) {
    parse_config();
    return 0;
}
```

</details>

---

**C13.** Viết hàm `print_float_bar(const char *label, float value, float max, int bar_width)` in thanh progress bar dạng ASCII — phổ biến trong embedded terminal UI.

```
Output mẫu:
Temp  : [████████░░░░░░░░░░░░] 40.0%
Battery: [████████████████░░░░] 80.0%
```

<details>
<summary>Xem đáp án</summary>

```c
#include <stdio.h>
#include <math.h>

void print_float_bar(const char *label, float value,
                     float max, int bar_width) {
    float pct = value / max;
    if (pct < 0.0f) pct = 0.0f;
    if (pct > 1.0f) pct = 1.0f;

    int filled = (int)(pct * bar_width + 0.5f);

    printf("%-10s: [", label);

    for (int i = 0; i < bar_width; i++) {
        printf("%s", i < filled ? "\xe2\x96\x88" : "\xe2\x96\x91");
        // █ = \xe2\x96\x88 (UTF-8)
        // ░ = \xe2\x96\x91 (UTF-8)
        // Nếu terminal không hỗ trợ UTF-8, dùng '#' và '.' thay thế
    }

    printf("] %.1f%%\n", pct * 100.0f);
}

int main(void) {
    print_float_bar("Temp",    40.0f, 100.0f, 20);
    print_float_bar("Battery", 80.0f, 100.0f, 20);
    print_float_bar("CPU",      3.2f,   5.0f, 20);
    return 0;
}
```

</details>

---

**C14.** Viết hàm `snprintf_safe` wrap lại `snprintf` — đảm bảo luôn null-terminate, trả về `0` nếu thành công, `-1` nếu bị truncate. Hữu ích trong embedded để phát hiện buffer quá nhỏ.

<details>
<summary>Xem đáp án</summary>

```c
#include <stdio.h>
#include <stdarg.h>

int snprintf_safe(char *buf, size_t size, const char *fmt, ...) {
    if (buf == NULL || size == 0) return -1;

    va_list args;
    va_start(args, fmt);
    int written = vsnprintf(buf, size, fmt, args);
    va_end(args);

    // vsnprintf trả về số ký tự SẼ được ghi nếu đủ chỗ
    // Nếu written >= size → bị truncate
    if (written < 0) {
        buf[0] = '\0';  // encoding error
        return -1;
    }

    if ((size_t)written >= size) {
        // Bị truncate — buf đã null-terminate bởi vsnprintf
        fprintf(stderr, "Warning: string truncated (%d > %zu)\n",
                written, size - 1);
        return -1;
    }

    return 0;  // thành công
}

// Test:
int main(void) {
    char buf[16];

    if (snprintf_safe(buf, sizeof(buf), "Hello %s", "World") == 0) {
        printf("OK: %s\n", buf);
    }

    if (snprintf_safe(buf, sizeof(buf), "Very long string that exceeds buffer") != 0) {
        printf("Truncated!\n");
    }

    return 0;
}
```

</details>

---

**C15.** Viết chương trình hoàn chỉnh đọc dữ liệu sensor giả lập (nhiệt độ float, ADC uint16_t) từ stdin, tính thống kê, in report ra stdout và log lỗi ra stderr. Format input mỗi dòng: `"TEMP=36.5,ADC=2048"`.

<details>
<summary>Xem đáp án</summary>

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <stdint.h>

typedef struct {
    float    temp;
    uint16_t adc;
} SensorData;

int parse_sensor_line(const char *line, SensorData *data) {
    float temp;
    unsigned int adc;
    if (sscanf(line, "TEMP=%f,ADC=%u", &temp, &adc) != 2) {
        return -1;
    }
    if (temp < -50.0f || temp > 150.0f) return -1;
    if (adc > 4095) return -1;  // 12-bit ADC

    data->temp = temp;
    data->adc  = (uint16_t)adc;
    return 0;
}

int main(void) {
    char line[64];
    float temp_sum = 0.0f;
    float temp_min = FLT_MAX;
    float temp_max = -FLT_MAX;
    int   count    = 0;
    int   errors   = 0;

    while (fgets(line, sizeof(line), stdin) != NULL) {
        line[strcspn(line, "\n")] = '\0';
        if (strlen(line) == 0) continue;

        SensorData d;
        if (parse_sensor_line(line, &d) < 0) {
            fprintf(stderr, "ERROR: invalid data: [%s]\n", line);
            errors++;
            continue;
        }

        temp_sum += d.temp;
        if (d.temp < temp_min) temp_min = d.temp;
        if (d.temp > temp_max) temp_max = d.temp;
        count++;
    }

    printf("\n=== Sensor Report ===\n");
    printf("Samples : %d\n", count);
    printf("Errors  : %d\n", errors);

    if (count > 0) {
        printf("Temp avg: %.2f C\n", temp_sum / count);
        printf("Temp min: %.2f C\n", temp_min);
        printf("Temp max: %.2f C\n", temp_max);
    } else {
        printf("No valid data\n");
    }

    return errors > 0 ? 1 : 0;
}
```

</details>

---

*— Hết GĐ2 Practice Questions —*

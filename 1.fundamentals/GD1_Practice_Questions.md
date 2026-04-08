# GĐ1 — Câu Hỏi Ôn Tập & Phỏng Vấn

> **Hướng dẫn:** Đọc câu hỏi, suy nghĩ kỹ, viết ra đáp án trước — rồi mới click **"Xem đáp án"** để kiểm tra.

---

## Mục Lục

- [Phần A — Lý Thuyết (Conceptual)](#phần-a--lý-thuyết-conceptual) — 25 câu
- [Phần B — Đọc Code (Code Reading)](#phần-b--đọc-code-code-reading) — 30 câu
- [Phần C — Viết Code (Coding)](#phần-c--viết-code-coding) — 20 câu

---

## Phần A — Lý Thuyết (Conceptual)

---

**A01.** `printf("Hello")` không có `\n`. Bạn đồng nghiệp nói *"chắc chắn sẽ in ra màn hình vì chương trình kết thúc bình thường"*. Câu này đúng hay sai? Giải thích đầy đủ cơ chế.

<details>
<summary>Xem đáp án</summary>

**Đúng trong trường hợp bình thường, nhưng không phải lúc nào cũng đúng.**

- `printf` ghi vào **buffer trong C runtime library (CRT)**, không ghi thẳng ra màn hình.
- Khi chương trình kết thúc bình thường (`return 0` hoặc `exit()`), CRT **tự động flush** tất cả buffer → "Hello" hiện ra.
- **Trường hợp KHÔNG in ra:** nếu chương trình crash (segfault, abort) trước khi return — buffer chưa flush → mất output.

```c
printf("Hello");
int *p = NULL;
*p = 42;  // crash → "Hello" không hiện
```

**Kết luận:** Đồng nghiệp sai ở chỗ "chắc chắn" — đúng với kết thúc bình thường, sai với crash.

**Debug tip:** Luôn thêm `\n` hoặc `fflush(stdout)`. Dùng `fprintf(stderr, ...)` vì stderr unbuffered.

</details>

---

**A02.** Giải thích sự khác nhau giữa `stdout` buffer trên Linux và Windows. Buffer đó nằm ở đâu trong bộ nhớ?

<details>
<summary>Xem đáp án</summary>

**Cơ chế buffer giống nhau** — đều nằm trong **C runtime library (CRT)**, trong RAM của process (heap hoặc data segment của CRT). Khác nhau ở bước cuối:

- **Linux:** CRT (glibc) → system call `write()` → Linux kernel → terminal
- **Windows:** CRT → Windows API (`WriteFile` / `WriteConsole`) → Windows kernel → console

Buffer **không nằm trong OS** — nó là vùng nhớ do thư viện C tạo ra, tồn tại trên mọi platform miễn là dùng C standard library.

</details>

---

**A03.** Tại sao C standard không quy định kích thước cố định cho `int`, `long`? Điều này gây ra vấn đề gì trong embedded và cách fix?

<details>
<summary>Xem đáp án</summary>

**Lý do thiết kế:** C được thiết kế để chạy trên mọi phần cứng — từ mainframe đến vi điều khiển 8-bit. Quy định cứng kích thước sẽ làm chậm hoặc không khả thi trên nhiều nền tảng. C chỉ quy định kích thước **tối thiểu**.

**Vấn đề trong embedded:**
- `long` trên Linux 64-bit = 8 bytes, Windows 64-bit = 4 bytes → cùng code, kết quả khác nhau
- `int` trên AVR = 2 bytes, trên x86 = 4 bytes → overflow khác nhau

**Fix:** Dùng `stdint.h`:
```c
uint8_t   // chắc chắn 8-bit unsigned
int16_t   // chắc chắn 16-bit signed
uint32_t  // chắc chắn 32-bit unsigned
```

</details>

---

**A04.** Cùng bit pattern `11001000`, nếu đọc là `uint8_t` ra bao nhiêu? Nếu đọc là `int8_t` ra bao nhiêu? CPU "biết" sự khác nhau này không?

<details>
<summary>Xem đáp án</summary>

- `uint8_t`: `128 + 64 + 8 = 200`
- `int8_t` (two's complement): MSB có trọng số `-128` → `-128 + 64 + 8 = -56`

**CPU không biết** — CPU chỉ thấy bit pattern. **Compiler** quyết định dùng instruction nào (signed hay unsigned) dựa trên kiểu dữ liệu bạn khai báo.

</details>

---

**A05.** Tại sao `char` là kiểu nguy hiểm trong embedded? Cho ví dụ bug cụ thể.

<details>
<summary>Xem đáp án</summary>

C standard không quy định `char` là signed hay unsigned — **do compiler quyết định**:
- ARM (embedded): `char` mặc định **unsigned**
- x86 (PC): `char` mặc định **signed**

**Bug cụ thể:**
```c
char c = 200;
if (c > 100) {
    // ARM: vào đây (200 > 100 ✅)
    // x86: KHÔNG vào (200 wrap → -56, -56 < 100 ❌)
}
```

Code chạy đúng trên PC, lên embedded bị sai hoàn toàn.

**Fix:** Dùng `uint8_t` hoặc `int8_t` thay vì `char` khi lưu số.

</details>

---

**A06.** Giải thích **Integer Promotion Rule**. Tại sao C standard thiết kế vậy? Nó có xảy ra trên CPU 8-bit (AVR Arduino) không?

<details>
<summary>Xem đáp án</summary>

**Rule:** Mọi operand có kiểu nhỏ hơn `int` (`uint8_t`, `uint16_t`, `char`, `short`) đều được **tự động promote lên `int`** trước khi tính toán.

**Lý do thiết kế:** CPU thực hiện phép tính trên register — register tối thiểu là kích thước "tự nhiên" của CPU (32-bit trên x86/ARM). C standard quy định promote lên `int` (kích thước tự nhiên) để phù hợp phần cứng.

**Trên AVR Arduino (CPU 8-bit):** **Có** — C standard bắt buộc, kể cả trên 8-bit. `int` trên AVR = 16-bit. Compiler ghép **2 register 8-bit** để giả lập 16-bit integer.

**Cách kiểm tra:** `printf("%zu bits\n", sizeof(int) * 8);`

</details>

---

**A07.** Phân biệt **Defined Behavior**, **Implementation-Defined Behavior**, và **Undefined Behavior**. Tại sao UB nguy hiểm hơn crash?

<details>
<summary>Xem đáp án</summary>

| Loại | Ý nghĩa | Ví dụ |
|---|---|---|
| **Defined** | C standard quy định rõ, mọi compiler ra cùng kết quả | `uint8_t` overflow wrap around |
| **Implementation-Defined** | Compiler chọn, phải document | `sizeof(int)` |
| **Undefined (UB)** | Không quy định gì — compiler làm gì cũng được | Shift ≥ số bit, chia cho 0 |

**Tại sao UB nguy hiểm hơn crash?**

Compiler **giả định UB không bao giờ xảy ra** và dùng giả định đó để tối ưu. Kết quả: compiler có thể **xóa cả đoạn code quan trọng** mà không báo lỗi:

```c
if (arr[i] == 1) {    // UB: out-of-bounds
    shutdown_system(); // compiler xóa luôn block này!
}
```

Crash ít nhất còn biết có vấn đề. UB: chương trình vẫn chạy, vẫn compile, nhưng thiếu code quan trọng — bug cực kỳ khó tìm.

</details>

---

**A08.** Sự khác nhau giữa **explicit cast** và **implicit conversion**? Cho ví dụ mỗi loại xảy ra lúc nào.

<details>
<summary>Xem đáp án</summary>

- **Explicit cast:** lập trình viên tự viết → `(int)x`
- **Implicit conversion:** compiler tự làm, không cần viết

**Implicit xảy ra ở 2 thời điểm:**

1. **Lúc tính toán (promotion):**
```c
uint8_t a = 250, b = 10;
int result = a + b;  // a, b tự promote lên int → 260
```

2. **Lúc gán (truncate):**
```c
uint8_t c = a + b;  // 260 tự truncate → 4
```

</details>

---

**A09.** `~` (bitwise NOT) có bị promotion không? Điều này gây ra bug gì trong embedded? Cách fix?

<details>
<summary>Xem đáp án</summary>

**Có** — `~` áp dụng promotion như mọi operator khác.

```c
uint8_t flags = 0xFF;
if (~flags) {          // promotion: ~0x000000FF = 0xFFFFFF00 ≠ 0 → luôn true!
    // chạy dù tất cả bit đã set — bug!
}
```

**Bug:** `~flags` không phải `0x00` (như kỳ vọng) mà là `0xFFFFFF00` (int 32-bit).

**Fix:**
```c
if ((uint8_t)~flags) { ... }
// hoặc
if (~flags & 0xFF)   { ... }
```

</details>

---

**A10.** Giải thích **Usual Arithmetic Conversions**. Khi nào `int` bị convert sang `unsigned`? Khi nào thì không?

<details>
<summary>Xem đáp án</summary>

Rule khi so sánh 2 kiểu khác nhau:

| Trường hợp | Kết quả |
|---|---|
| `uint8_t` vs `int` | `uint8_t` promote lên `int` → cả 2 **signed** |
| `uint16_t` vs `int` | `uint16_t` promote lên `int` → cả 2 **signed** |
| `uint32_t` vs `int` | `int` convert sang `uint32_t` → cả 2 **unsigned** ← trap! |

**`int` chỉ bị ép sang unsigned khi gặp `uint32_t` trở lên** (lớn hơn hoặc bằng `int`). `uint8_t` và `uint16_t` nhỏ hơn `int` nên chúng promote lên, không phải `int` bị ép xuống.

</details>

---

**A11.** Tại sao `goto` bị coi là "xấu" trong lập trình tổng quát, nhưng lại được dùng rộng rãi trong Linux kernel? Nêu điều kiện để dùng `goto` đúng cách.

<details>
<summary>Xem đáp án</summary>

**Tại sao "xấu":** Dùng bừa bãi → code nhảy lung tung, không trace được flow → "spaghetti code".

**Tại sao Linux kernel dùng:** Pattern **error handling + cleanup** — khi có nhiều bước khởi tạo, mỗi bước có thể fail và cần cleanup theo thứ tự ngược lại:

```c
int init(void) {
    if (step1() < 0) goto cleanup1;
    if (step2() < 0) goto cleanup2;
    return 0;
cleanup2: undo_step1();
cleanup1: return -1;
}
```

Labels cascade (fall through) → không cần lặp lại cleanup code.

**Điều kiện dùng đúng cách:**
1. Chỉ nhảy **xuống** (forward), không nhảy ngược lên
2. Chỉ dùng cho error handling / cleanup
3. Label đặt cuối function, rõ ràng

</details>

---

**A12.** Tại sao macro nhiều dòng phải dùng `do { } while(0)` thay vì chỉ `{ }`? Cho ví dụ bug cụ thể nếu không dùng.

<details>
<summary>Xem đáp án</summary>

**Bug với `{ }` đơn thuần:**
```c
#define RESET() { clear(); init(); }

if (error)
    RESET();   // expand: { clear(); init(); };  ← dấu ; thừa!
else           // compiler error: else không có if tương ứng
    run();
```

**Fix với `do { } while(0)`:**
```c
#define RESET() do { clear(); init(); } while(0)

if (error)
    RESET();   // expand: do { clear(); init(); } while(0);  ← hợp lệ
else
    run();
```

`do { } while(0)` là một **statement hoàn chỉnh** — nhận dấu `;` mà không gây lỗi cú pháp.

</details>

---

**A13.** `sizeof` trả về kiểu gì? Tại sao điều này quan trọng trong embedded? Cho ví dụ bug phổ biến.

<details>
<summary>Xem đáp án</summary>

`sizeof` trả về **`size_t`** — kiểu unsigned, kích thước phụ thuộc kiến trúc (32-bit → `unsigned int`, 64-bit → `unsigned long`).

**Bug phổ biến — subtraction với unsigned:**
```c
size_t n = sizeof(arr) / sizeof(arr[0]);  // n = 5

if (n - 6 > 0) {  // 5 - 6 với unsigned → wrap → 4294967295 > 0 → true!
    // vào đây dù logic sai
}
```

**Fix:**
```c
if ((int)n - 6 > 0) { ... }  // cast về signed trước khi trừ
if (n > 6)           { ... }  // hoặc đổi chiều so sánh
```

</details>

---

**A14.** Phân biệt vòng đời và vùng nhớ của: biến `auto` local, biến `static` local, biến `static` global, biến `extern`.

<details>
<summary>Xem đáp án</summary>

| | Vùng nhớ | Vòng đời | Visibility |
|---|---|---|---|
| `auto` local | Stack | Từ khi khai báo đến khi ra khỏi scope | Chỉ trong block |
| `static` local | Data segment | Suốt chương trình | Chỉ trong function |
| `static` global | Data segment | Suốt chương trình | Chỉ trong file (internal linkage) |
| `extern` | Data segment (định nghĩa ở file khác) | Suốt chương trình | Toàn bộ chương trình |

**Điểm quan trọng:** `static` local chỉ khởi tạo **một lần** khi chương trình khởi động, không phải mỗi lần gọi hàm.

</details>

---

**A15.** `const int x = 10` khai báo trong function nằm ở đâu? `const int MAX = 100` khai báo global nằm ở đâu? Tại sao khác nhau?

<details>
<summary>Xem đáp án</summary>

- `const int x = 10` **local** → **Stack** — compiler enforce không ghi đè trong code, nhưng vẫn là local variable
- `const int MAX = 100` **global** → **Text segment (.rodata)** — read-only ở mức MMU, ghi vào = segfault

**Tại sao global const vào Text segment?**
1. Compiler thấy giá trị không đổi → đặt vào `.rodata`
2. Linker gộp `.rodata` + `.text` → Text segment
3. OS đánh dấu read-only qua MMU

**Lợi ích:** Nhiều instance của cùng chương trình dùng chung 1 bản Text segment (memory mapping) → tiết kiệm RAM.

</details>

---

**A16.** Tại sao `++i` và `i++` trong cùng một expression là Undefined Behavior? Compiler có thể làm gì với UB đó?

<details>
<summary>Xem đáp án</summary>

C standard **không quy định thứ tự tính argument của hàm**. Trong `printf("%d %d", i++, ++i)`, compiler được phép tính `++i` trước hoặc `i++` trước.

Khi có 2 side effect trên cùng biến `i` mà không có **sequence point** giữa chúng → **Undefined Behavior**.

**Compiler có thể:**
- Ra kết quả khác nhau tùy optimization flag (`-O0` vs `-O2`)
- Xóa cả expression
- Ra kết quả khác nhau giữa GCC và Clang

**Fix:** Tách ra 2 statement riêng biệt:
```c
printf("%d ", i++);
printf("%d\n", ++i);
```

</details>

---

**A17.** Giải thích **stack frame**. Khi `main()` gọi `foo()`, gọi `bar()` — điều gì xảy ra trên stack? SP (Stack Pointer) di chuyển thế nào?

<details>
<summary>Xem đáp án</summary>

Mỗi lần gọi hàm, CPU tạo **stack frame** chứa: biến local, tham số, return address, saved registers.

```
main() gọi foo() gọi bar():

High address
┌─────────────┐
│ main frame  │ ← SP ban đầu
├─────────────┤
│  foo frame  │ ← SP sau khi main gọi foo
├─────────────┤
│  bar frame  │ ← SP sau khi foo gọi bar (SP thấp nhất)
└─────────────┘
Low address
```

- **Gọi hàm:** SP giảm (stack grow downward), push frame mới
- **Return:** SP tăng, pop frame, nhảy về return address

**Code của hàm** (instructions) nằm trong **Text segment** — không nằm trên stack. Stack chỉ chứa **data khi chạy**.

Thực tế trước `main()` còn có `_start()` do C runtime tạo — khởi tạo BSS, argc/argv, rồi mới gọi `main()`.

</details>

---

**A18.** Phân biệt **Data segment** và **BSS segment**. Tại sao OS cần phân biệt 2 vùng này?

<details>
<summary>Xem đáp án</summary>

- **Data segment:** biến global/static **đã khởi tạo** có giá trị khác 0 → giá trị phải lưu trong file binary
- **BSS segment:** biến global/static **chưa khởi tạo** hoặc khởi tạo = 0 → file binary **không cần lưu giá trị**, chỉ lưu kích thước

**Lý do phân biệt:** Tiết kiệm kích thước file binary. Nếu có `int arr[10000] = {0}`, Data segment phải lưu 40000 bytes trong file. BSS chỉ cần lưu "40000 bytes, tất cả = 0" → OS tự cấp và zero-fill khi load chương trình.

Quan trọng trong embedded: flash memory giới hạn, BSS giúp giảm kích thước firmware.

</details>

---

**A19.** `for (int i = 0; i < n; i++)` vs `for (size_t i = 0; i < n; i++)` — cái nào đúng hơn? Khi nào dùng cái nào?

<details>
<summary>Xem đáp án</summary>

- `int i`: phổ biến, an toàn khi `n` là `int`. Có thể dùng `i > 0` để kiểm tra sau khi trừ.
- `size_t i`: đúng về mặt type khi `n` là `size_t` (như `sizeof` / số phần tử mảng). Nhưng nguy hiểm khi trừ:

```c
size_t n = 5;
for (size_t i = n - 1; i >= 0; i--) {  // bug vô tận! i >= 0 luôn true với unsigned
    ...
}
```

**Khi nào dùng:**
- Dùng `int` khi cần loop ngược hoặc có thể trừ
- Dùng `size_t` khi loop xuôi qua mảng, không trừ, muốn type-correct với `sizeof`
- Trong embedded: thường dùng `int` hoặc `uint32_t` cho rõ ràng

</details>

---

**A20.** Giải thích tại sao `do { } while(0)` macro chạy **0 lần** dù body luôn được thực thi ít nhất 1 lần?

<details>
<summary>Xem đáp án</summary>

Đây là câu hỏi đánh lừa — **không mâu thuẫn**:

- `do { body } while(0)`: body chạy **đúng 1 lần**, sau đó kiểm tra `0` (false) → thoát ngay.
- "Chạy 0 lần" ý nói vòng lặp không lặp lại — chỉ chạy **1 lần rồi dừng**, không phải 0 lần.

Mục đích của pattern này **không phải để lặp** mà để tạo một **compound statement** có thể nhận dấu `;` an toàn trong macro — behavior của vòng lặp là phụ.

</details>

---

**A21.** Tại sao Bubble Sort có độ phức tạp O(n²) dù có thể tối ưu để dừng sớm khi mảng đã sort?

<details>
<summary>Xem đáp án</summary>

Big-O biểu thị **worst case** (hoặc average case) — không phải best case.

- **Best case** (mảng đã sort, có early-exit optimization): O(n) — chỉ duyệt 1 lần, không swap
- **Worst case** (mảng sort ngược): O(n²) — mọi cặp đều cần swap

Big-O convention dùng **worst case** → O(n²).

**Early-exit optimization:**
```c
for (int i = 0; i < n; i++) {
    int swapped = 0;
    for (int j = 0; j < n - i - 1; j++) {
        if (arr[j] > arr[j+1]) {
            swap(&arr[j], &arr[j+1]);
            swapped = 1;
        }
    }
    if (!swapped) break;  // mảng đã sort, dừng sớm
}
```

Dù có optimization, worst case vẫn O(n²).

</details>

---

**A22.** Khi nhiều instance của cùng chương trình chạy đồng thời, vùng nhớ nào dùng chung, vùng nào độc lập? Tại sao?

<details>
<summary>Xem đáp án</summary>

| Vùng | Dùng chung hay độc lập | Lý do |
|---|---|---|
| Text segment | **Dùng chung** | Read-only → OS map nhiều process vào cùng vùng vật lý qua MMU |
| Data/BSS | **Độc lập** | Mỗi process có state riêng |
| Heap | **Độc lập** | Dynamic allocation riêng |
| Stack | **Độc lập** | Local variables riêng |

**Lợi ích dùng chung Text segment:** 10 instance cùng chương trình → chỉ cần 1 bản code trong RAM thay vì 10 bản → tiết kiệm đáng kể RAM vật lý.

</details>

---

**A23.** Phân biệt `break` trong `switch` và `break` trong vòng lặp. Nếu có `switch` lồng trong `for`, `break` thoát khỏi cái nào?

<details>
<summary>Xem đáp án</summary>

`break` luôn thoát khỏi **cấu trúc gần nhất bao quanh nó** — `switch`, `for`, `while`, hoặc `do-while`.

```c
for (int i = 0; i < 10; i++) {
    switch (i) {
        case 3:
            break;  // thoát khỏi switch, KHÔNG thoát khỏi for
    }
    printf("%d\n", i);  // vẫn chạy
}
```

Nếu muốn thoát khỏi `for` từ bên trong `switch`: dùng `goto` hoặc flag biến:
```c
int done = 0;
for (int i = 0; i < 10 && !done; i++) {
    switch (i) {
        case 3: done = 1; break;
    }
}
```

</details>

---

**A24.** `static` trong function và `static` ở file scope khác nhau thế nào về linkage?

<details>
<summary>Xem đáp án</summary>

- **`static` trong function:** giới hạn **vòng đời** (lifetime) — biến tồn tại suốt chương trình nhưng chỉ truy cập được từ trong function đó. Không liên quan đến linkage.

- **`static` ở file scope (global):** giới hạn **linkage** — biến/function chỉ visible trong file đó (**internal linkage**). File khác không thể `extern` để dùng.

```c
// file_a.c
static int counter = 0;  // file_b.c không thể truy cập

// file_b.c
extern int counter;  // lỗi! linker không tìm thấy
```

Trong embedded, `static` file scope thường dùng để **encapsulate** — giống `private` trong OOP.

</details>

---

**A25.** Linear Search O(n) và Binary Search O(log n) — khi nào nên dùng Linear Search dù chậm hơn?

<details>
<summary>Xem đáp án</summary>

**Dùng Linear Search khi:**
1. **Mảng chưa sort** — Binary Search yêu cầu mảng đã sort
2. **Mảng nhỏ** — overhead của sort > lợi ích của Binary Search
3. **Tìm tất cả** phần tử thỏa điều kiện, không chỉ một
4. **Dữ liệu linked list** — Binary Search không hiệu quả vì không random access
5. **Embedded với RAM giới hạn** — sort tốn thêm bộ nhớ

**Quan trọng:** O(log n) tốt hơn O(n) chỉ khi n đủ lớn. Với n = 10, 2 cách gần như bằng nhau.

</details>

---

## Phần B — Đọc Code (Code Reading)

---

**B01.** Output của đoạn code này là gì? Giải thích từng bước.

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint8_t a = 200;
    uint8_t b = 100;
    uint8_t c = a + b;
    int     d = a + b;
    printf("c=%u d=%d\n", c, d);
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

**Output:** `c=44 d=300`

**Giải thích:**
1. `a`, `b` là `uint8_t` → promote lên `int` trước khi cộng
2. `int + int = int` → `200 + 100 = 300`
3. Gán vào `c` (`uint8_t`) → truncate: `300 mod 256 = 44`
4. Gán vào `d` (`int`) → `300` vừa trong `int` → `d = 300`

</details>

---

**B02.** Đoạn code này có bug không? Nếu có, bug ở đâu và output thực tế là gì?

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint8_t flags = 0xFF;
    if (~flags) {
        printf("co bit chua set\n");
    } else {
        printf("tat ca bit da set\n");
    }
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

**Có bug.** Output thực tế: `"co bit chua set"` — **sai** về mặt logic.

**Giải thích:**
1. `flags = 0xFF = 0x000000FF` (sau promote lên int)
2. `~flags = ~0x000000FF = 0xFFFFFF00` (flip 32-bit)
3. `0xFFFFFF00 != 0` → điều kiện **true** → vào if

**Fix:**
```c
if ((uint8_t)~flags) { ... }
// hoặc
if (~flags & 0xFF)   { ... }
```

Với fix: `(uint8_t)0xFFFFFF00 = 0x00 = 0` → false → vào else → đúng.

</details>

---

**B03.** Output là gì?

```c
#include <stdio.h>

void foo(void) {
    static int count = 0;
    int local = 0;
    count++;
    local++;
    printf("count=%d local=%d\n", count, local);
}

int main(void) {
    foo();
    foo();
    foo();
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

```
count=1 local=1
count=2 local=1
count=3 local=1
```

- `count`: static → khởi tạo 1 lần, giữ giá trị qua các lần gọi
- `local`: auto → tạo mới mỗi lần gọi, luôn bắt đầu từ 0

</details>

---

**B04.** Đoạn code này in ra gì? Có Undefined Behavior không?

```c
#include <stdio.h>

int main(void) {
    int i = 5;
    printf("%d %d\n", i++, ++i);
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

**Undefined Behavior** — không thể dự đoán output.

C standard không quy định thứ tự tính argument của hàm. Có 2 side effect trên cùng biến `i` mà không có sequence point → UB.

Thực tế các compiler ra kết quả khác nhau:
- GCC `-O0`: có thể ra `5 7`
- Clang: có thể ra `6 6`

**Không có đáp án đúng duy nhất** — đây là UB.

</details>

---

**B05.** Output là gì? Giải thích tại sao.

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint32_t x = 1;
    int      y = -1;
    if (x > y) {
        printf("x lon hon y\n");
    } else {
        printf("x nho hon hoac bang y\n");
    }
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

**Output:** `"x nho hon hoac bang y"`

**Giải thích (Usual Arithmetic Conversions):**
- `uint32_t` vs `int` → `int` bị convert sang `uint32_t`
- `-1` dạng bit (32-bit): `0xFFFFFFFF` → unsigned = `4294967295`
- So sánh: `1 > 4294967295` → **false**

Đây là trap kinh điển — khác với `uint8_t` vs `int` (uint8_t promote lên int, cả 2 signed).

</details>

---

**B06.** Đoạn code này có vấn đề gì không?

```c
#include <stdio.h>

int main(void) {
    int arr[5] = {1, 2, 3, 4, 5};
    int n = 5;

    for (int i = n; i >= 0; i--) {
        printf("%d\n", arr[i]);
    }
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

**Có bug — out-of-bounds access (Undefined Behavior).**

Vòng lặp bắt đầu từ `i = n = 5`, nhưng index hợp lệ của `arr` là `0` đến `4`. `arr[5]` là **ngoài mảng** → UB.

**Fix:**
```c
for (int i = n - 1; i >= 0; i--) {
    printf("%d\n", arr[i]);
}
```

</details>

---

**B07.** Output là gì?

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint8_t x = 0x01;
    uint8_t result = x << 7;
    printf("%u\n", result);
    printf("%u\n", x << 7);
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

**Output:**
```
128
128
```

**Giải thích:**
- `x << 7`: `0x01` promote lên `int` → `0x00000001` → shift 7 → `0x00000080 = 128`
- Gán vào `uint8_t result`: `128` vừa trong 8-bit → `result = 128`
- `printf("%u", x << 7)`: biểu thức `x << 7` là `int` → `128` (vẫn fit trong unsigned)

Cả 2 đều `128` vì `0x80` vừa trong cả `uint8_t` lẫn `int`.

</details>

---

**B08.** Đoạn code này làm gì? Có bug không?

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    int arr[] = {10, 20, 30, 40, 50};
    size_t n = sizeof(arr) / sizeof(arr[0]);

    for (size_t i = n - 1; i >= 0; i--) {
        printf("%d\n", arr[i]);
    }
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

**Bug nghiêm trọng — vòng lặp vô tận, sau đó crash.**

`i` là `size_t` (unsigned). Khi `i = 0` và thực hiện `i--`:
- `0 - 1` với unsigned → wrap around → `SIZE_MAX` (số rất lớn)
- `SIZE_MAX >= 0` → **luôn true** → vòng lặp vô tận
- Truy cập `arr[SIZE_MAX]` → out-of-bounds → crash (segfault)

**Fix:**
```c
for (int i = (int)n - 1; i >= 0; i--) {
    printf("%d\n", arr[i]);
}
```

</details>

---

**B09.** Output là gì? Có gì đặc biệt ở đây?

```c
#include <stdio.h>

int main(void) {
    int x = 5;
    int y = 10;
    int result = (x > y) ? x++ : y++;
    printf("result=%d x=%d y=%d\n", result, x, y);
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

**Output:** `result=10 x=5 y=11`

**Giải thích:**
- `x > y` → `5 > 10` → false → vào vế phải `y++`
- `y++`: trả về giá trị **trước** khi tăng → `result = 10`
- Sau đó `y` tăng lên `11`
- `x` không bị đụng → `x = 5`

</details>

---

**B10.** Đoạn code dưới có bug không? Output là gì?

```c
#include <stdio.h>

int main(void) {
    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) continue;
        if (i == 7) break;
        printf("%d ", i);
    }
    printf("\n");
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

**Không bug.** Output: `1 3 5`

**Giải thích:**
- `i=0`: `0%2==0` → `continue` (skip)
- `i=1`: in `1`
- `i=2`: `continue`
- `i=3`: in `3`
- `i=4`: `continue`
- `i=5`: in `5`
- `i=6`: `continue`
- `i=7`: `break` → thoát (không in `7`)

</details>

---

**B11.** Output là gì? Tại sao kết quả có vẻ "vô lý"?

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    int a = -1;
    if (a < sizeof(int)) {
        printf("nho hon\n");
    } else {
        printf("lon hon hoac bang\n");
    }
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

**Output:** `"lon hon hoac bang"` — vô lý vì `-1 < 4` đúng về mặt toán học!

**Giải thích:**
- `sizeof(int)` trả về `size_t` (unsigned)
- So sánh `int` vs `size_t` → `int` convert sang `size_t`
- `-1` → unsigned = `4294967295`
- `4294967295 < 4` → **false**

**Fix:**
```c
if (a < (int)sizeof(int)) { ... }
```

</details>

---

**B12.** Đoạn code này làm gì khi chạy? Giải thích flow của `goto`.

```c
#include <stdio.h>

int setup(void) {
    printf("open port\n");
    if (1) goto fail_port;   // giả lập lỗi
    printf("alloc buffer\n");
    return 0;

fail_port:
    printf("cleanup port\n");
    return -1;
}

int main(void) {
    int r = setup();
    printf("result = %d\n", r);
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

**Output:**
```
open port
cleanup port
result = -1
```

**Flow:**
1. In "open port"
2. `goto fail_port` → nhảy thẳng xuống label `fail_port`
3. "alloc buffer" bị **bỏ qua hoàn toàn**
4. In "cleanup port"
5. Return -1

</details>

---

**B13.** Output là gì?

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint8_t a = 10;
    uint8_t b = 20;
    uint8_t c = (a > b) ? a : a + b;
    printf("%u\n", c);

    a = 150;
    b = 150;
    c = (a > b) ? a : a + b;
    printf("%u\n", c);
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

**Output:**
```
30
44
```

**Giải thích:**
- Lần 1: `a > b` → false → `a + b = 10 + 20 = 30` (int) → gán uint8_t → `30` (vừa)
- Lần 2: `a > b` → false (`150 == 150`) → `a + b = 150 + 150 = 300` (int) → gán uint8_t → `300 mod 256 = 44`

</details>

---

**B14.** Tìm tất cả vấn đề trong đoạn code này:

```c
#include <stdio.h>

#define SQUARE(x) x * x

int main(void) {
    int a = 3;
    int b = SQUARE(a + 1);
    printf("%d\n", b);
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

**Bug — macro expansion sai.**

`SQUARE(a + 1)` expand thành `a + 1 * a + 1` = `3 + 1 * 3 + 1 = 7` (không phải `16`).

**Output:** `7` (kỳ vọng `16`)

**Fix — luôn dùng ngoặc trong macro:**
```c
#define SQUARE(x) ((x) * (x))
```

Expand: `((a + 1) * (a + 1))` = `4 * 4 = 16` ✅

**Vẫn còn vấn đề với `SQUARE(i++)` — double evaluation.** Dùng `static inline` thay vì macro:
```c
static inline int square(int x) { return x * x; }
```

</details>

---

**B15.** Output là gì? Giải thích bit-level.

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint8_t x = 0b10110100;  // 180
    uint8_t result;

    result = x & 0x0F;   printf("A: %u\n", result);
    result = x | 0x0F;   printf("B: %u\n", result);
    result = x ^ 0xFF;   printf("C: %u\n", result);
    result = x >> 4;     printf("D: %u\n", result);
    result = (x >> 4) | (x << 4);  printf("E: %u\n", result);
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

`x = 1011 0100 = 180`

- **A:** `x & 0x0F` = `1011 0100 & 0000 1111 = 0000 0100 = 4` (lấy nibble thấp)
- **B:** `x | 0x0F` = `1011 0100 | 0000 1111 = 1011 1111 = 191` (set nibble thấp)
- **C:** `x ^ 0xFF` = `1011 0100 ^ 1111 1111 = 0100 1011 = 75` (flip tất cả bit = `~x`)
- **D:** `x >> 4` = `0000 1011 = 11` (lấy nibble cao)
- **E:** `(x >> 4) | (x << 4)` = rotate 4-bit = `0100 1011 = 75` (swap 2 nibble)

**Output:**
```
A: 4
B: 191
C: 75
D: 11
E: 75
```

</details>

---

**B16.** Đoạn code này sort đúng không? Nếu sai, tại sao?

```c
void bubble_sort(int arr[], int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                int tmp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = tmp;
            }
        }
    }
}
```

<details>
<summary>Xem đáp án</summary>

**Sort đúng nhưng không tối ưu.**

Thiếu `- i` trong điều kiện inner loop: `j < n - 1` thay vì `j < n - i - 1`.

**Hậu quả:** Vẫn ra kết quả đúng (vì vẫn so sánh đủ các cặp), nhưng tiếp tục so sánh các phần tử đã về đúng vị trí → **thừa phép tính**.

Phiên bản đúng và tối ưu:
```c
for (int j = 0; j < n - i - 1; j++) { ... }
```

</details>

---

**B17.** Output là gì?

```c
#include <stdio.h>

int main(void) {
    int i = 0;
    do {
        printf("%d ", i);
        i++;
    } while (i < 3);
    printf("\n");

    i = 5;
    do {
        printf("%d ", i);
        i++;
    } while (i < 3);
    printf("\n");
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

**Output:**
```
0 1 2
5
```

- Lần 1: bình thường, in 0 1 2
- Lần 2: `i = 5`, điều kiện `5 < 3` = false, nhưng `do-while` **chạy body trước** → in `5` → kiểm tra điều kiện → false → thoát

`do-while` luôn chạy ít nhất 1 lần.

</details>

---

**B18.** Đoạn code này in gì? Có bug không?

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint8_t x = 0x05;
    uint8_t y = x << 8;
    printf("%u\n", y);
    printf("%u\n", x << 8);
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

**Output:**
```
0
1280
```

**Giải thích:**
- `x << 8`: `uint8_t` promote lên `int` → `0x00000005` → shift 8 → `0x00000500 = 1280`
- Gán vào `uint8_t y`: truncate → `0x00 = 0`
- `printf("%u", x << 8)`: biểu thức là `int` = `1280` → in trực tiếp

</details>

---

**B19.** Tìm bug trong đoạn code này:

```c
#include <stdio.h>

int linear_search(int arr[], int n, int target) {
    for (int i = 0; i <= n; i++) {  // chú ý điều kiện
        if (arr[i] == target) return i;
    }
    return -1;
}

int main(void) {
    int arr[] = {1, 2, 3, 4, 5};
    printf("%d\n", linear_search(arr, 5, 6));
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

**Bug — off-by-one error dẫn đến out-of-bounds (UB).**

Điều kiện `i <= n` cho phép `i = 5` — nhưng index hợp lệ chỉ là `0` đến `4`. `arr[5]` là UB.

**Fix:**
```c
for (int i = 0; i < n; i++) { ... }
```

</details>

---

**B20.** Output là gì? Giải thích tại sao `const` global không thể ghi.

```c
#include <stdio.h>

const int MAX = 100;

int main(void) {
    int *p = (int *)&MAX;
    *p = 200;  // cố tình ghi vào const
    printf("%d\n", MAX);
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

**Undefined Behavior — thường crash (Segmentation Fault).**

`const int MAX` global nằm trong **Text segment (.rodata)** — OS đánh dấu read-only ở mức MMU. Ghi vào → hardware protection violation → **segfault**.

Compiler có thể warn: `cast discards 'const' qualifier`.

Ngay cả nếu không crash, đây vẫn là UB theo C standard — compiler có thể optimize `MAX` thành literal `100` ở mọi nơi dùng, không thèm đọc lại từ memory.

</details>

---

**B21.** Đoạn code này dùng `goto` đúng cách không? Nếu có lỗi ở `step2`, cleanup diễn ra thế nào?

```c
int init(void) {
    if (open_port()    < 0) goto err_port;
    if (alloc_buffer() < 0) goto err_buf;
    if (start_dma()    < 0) goto err_dma;
    return 0;

err_dma:  stop_dma();
err_buf:  free_buffer();
err_port: close_port();
          return -1;
}
```

<details>
<summary>Xem đáp án</summary>

**Đúng cách — đây là pattern chuẩn trong Linux kernel.**

Nếu `alloc_buffer()` fail → nhảy đến `err_buf`:
- Chạy `free_buffer()` (undo alloc_buffer)
- **Fall through** xuống `err_port`
- Chạy `close_port()` (undo open_port)
- Return -1

`stop_dma()` **không chạy** vì DMA chưa start — đúng logic cleanup.

Pattern: nhảy vào đúng điểm, các bước cleanup bên dưới tự cascade.

</details>

---

**B22.** Output là gì?

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint8_t  a = 200;
    uint16_t b = 300;
    uint32_t c = 70000;

    printf("%zu\n", sizeof(a + b));
    printf("%zu\n", sizeof(b + c));
    printf("%zu\n", sizeof(a + c));
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

**Output:**
```
4
4
4
```

**Giải thích:**
- `a + b`: `uint8_t` và `uint16_t` đều nhỏ hơn `int` → cả 2 promote lên `int` (4 bytes)
- `b + c`: `uint16_t` promote lên `int`, nhưng `uint32_t` ≥ `int` → result là `uint32_t` (4 bytes)
- `a + c`: `uint8_t` promote lên `int`, nhưng `uint32_t` ≥ `int` → result là `uint32_t` (4 bytes)

`sizeof` tính trên kiểu của expression, không tính giá trị → tất cả ra 4 bytes.

</details>

---

**B23.** Tìm bug và giải thích:

```c
#include <stdio.h>

#define MAX(a, b) (a > b ? a : b)

int main(void) {
    int i = 3, j = 5;
    int m = MAX(i++, j++);
    printf("m=%d i=%d j=%d\n", m, i, j);
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

**Bug — double evaluation trong macro.**

`MAX(i++, j++)` expand thành `(i++ > j++ ? i++ : j++)`.

`j` được tính **2 lần** (`j++` trong so sánh và `j++` trong vế false) → **Undefined Behavior**.

Thực tế trên nhiều compiler: `m=6 i=4 j=7` — nhưng không thể tin vào kết quả này.

**Fix — dùng `static inline`:**
```c
static inline int max(int a, int b) { return a > b ? a : b; }
```

</details>

---

**B24.** Output là gì? Giải thích từng dòng.

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint8_t x = 0xFF;
    printf("%u\n",   x);
    printf("%u\n",  ~x);
    printf("%u\n",  (uint8_t)~x);
    printf("%d\n",  (int8_t)x);
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

**Output:**
```
255
4294967040
0
-1
```

- `x = 0xFF = 255` (uint8_t)
- `~x`: promote lên int → `~0x000000FF = 0xFFFFFF00 = 4294967040`
- `(uint8_t)~x`: truncate `0xFFFFFF00` → `0x00 = 0`
- `(int8_t)x`: đọc `0xFF` như signed 8-bit → `-1` (two's complement)

</details>

---

**B25.** Đoạn code dưới đây in gì? Và tại sao đây là bad practice?

```c
#include <stdio.h>
#include <stdint.h>

static int counter = 0;

int* get_counter(void) {
    return &counter;
}

int main(void) {
    int *p = get_counter();
    *p = 100;
    printf("%d\n", counter);
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

**Output:** `100`

**Tại sao bad practice:**
1. **Phá vỡ encapsulation** — biến `static` dùng internal linkage chính là để ẩn đi, nhưng trả về pointer lại cho bên ngoài ghi tự do.
2. **Không kiểm soát được** — caller có thể ghi bất kỳ giá trị nào, kể cả giá trị không hợp lệ.
3. **Lifetime hazard** — nếu sau này đổi `counter` sang local variable, pointer dangling ngay.

**Fix đúng cách:**
```c
void set_counter(int val) { counter = val; }
int  get_counter_val(void) { return counter; }
```

</details>

---

**B26.** Output là gì? Giải thích tại sao `b` không phải 0.

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint8_t a = 1;
    uint8_t b = a - 2;
    printf("%u\n", b);
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

**Output:** `255`

**Giải thích:**
1. `a = 1`, `2` là int literal
2. `a - 2`: `a` promote lên `int` → `1 - 2 = -1` (int)
3. Gán vào `uint8_t b`: `-1` dạng bit = `0xFFFFFFFF` → truncate 8-bit → `0xFF = 255`

Đây là **unsigned wraparound** — không phải UB (khác với signed overflow). Unsigned underflow/overflow là defined behavior trong C.

</details>

---

**B27.** Linear search dưới đây có vấn đề gì với performance?

```c
int search(int *arr, int n, int target) {
    for (int i = 0; i < n; i++) {
        if (arr[i] == target) {
            return i;
        }
    }
    return -1;
}

// Gọi như sau:
for (int q = 0; q < 1000; q++) {
    search(arr, 1000000, queries[q]);
}
```

<details>
<summary>Xem đáp án</summary>

**Vấn đề:** 1000 queries × 1,000,000 phần tử = **1 tỷ phép so sánh** worst case → O(n × q).

**Cải thiện:**
1. **Sort trước + Binary Search:** Sort O(n log n) một lần → mỗi query O(log n) → tổng O(n log n + q log n)
2. **Hash table:** Build O(n) một lần → mỗi query O(1) → tổng O(n + q)

**Trong embedded** với n nhỏ (< 100): Linear Search vẫn ổn vì overhead của sort/hash không đáng.

</details>

---

**B28.** Đoạn code này có Undefined Behavior không?

```c
#include <stdint.h>

int main(void) {
    uint8_t x = 0x01;
    uint8_t y = x << 8;   // (1)
    uint32_t z = x << 31; // (2)
    uint32_t w = x << 32; // (3)
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

- **(1) `x << 8`:** `uint8_t` promote lên `int` (32-bit) → shift 8 → OK, **defined**
- **(2) `x << 31`:** promote lên `int` → `0x00000001 << 31 = 0x80000000` → đây là **UB** nếu `int` là signed và kết quả overflow! Shifting into sign bit của signed integer = UB.
- **(3) `x << 32`:** shift amount ≥ số bit của kiểu (32-bit int) → **UB**

**Điểm tinh tế:** Nếu dùng `uint32_t x` thay vì `uint8_t`: `(2)` trở thành OK (unsigned không có signed overflow UB).

</details>

---

**B29.** Output là gì?

```c
#include <stdio.h>

int main(void) {
    int x = 10;
    int y = (x > 5) ? (x = 20, x + 1) : (x = 0, x - 1);
    printf("x=%d y=%d\n", x, y);
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

**Output:** `x=20 y=21`

**Giải thích:**
- `x > 5` → true → vào vế trái: `(x = 20, x + 1)`
- Comma operator: tính `x = 20` trước, rồi tính `x + 1 = 21` → `y = 21`
- `x` đã bị gán thành `20`

Comma operator trong C: tính từ trái sang phải, trả về giá trị của expression **cuối cùng**.

</details>

---

**B30.** Tìm tất cả vấn đề trong đoạn code này:

```c
#include <stdio.h>
#include <stdint.h>

void process(uint8_t *data, int len) {
    for (int i = 0; i <= len; i++) {
        if (data[i] & 0x80) {
            data[i] = ~data[i];
        }
    }
}

int main(void) {
    uint8_t buf[4] = {0x10, 0x90, 0x20, 0xA0};
    process(buf, 4);
    for (int i = 0; i < 4; i++) {
        printf("%02X ", buf[i]);
    }
    return 0;
}
```

<details>
<summary>Xem đáp án</summary>

**2 vấn đề:**

1. **Off-by-one:** `i <= len` → khi `i = 4`, `data[4]` là out-of-bounds (UB). Fix: `i < len`.

2. **Promotion trap với `~`:** `data[i] = ~data[i]` — `~data[i]` là `int` 32-bit. Gán về `uint8_t` → truncate đúng, nhưng compiler có thể warn. Fix: `data[i] = (uint8_t)~data[i]`.

**Output (nếu sửa bug #1):** `10 6F 20 5F`
- `0x10`: bit 7 = 0 → không flip → `0x10`
- `0x90`: bit 7 = 1 → flip → `~0x90 = 0x6F`
- `0x20`: bit 7 = 0 → không flip → `0x20`
- `0xA0`: bit 7 = 1 → flip → `~0xA0 = 0x5F`

</details>

---

## Phần C — Viết Code (Coding)

---

**C01.** Viết hàm `count_bits(uint8_t x)` trả về số bit `1` trong `x`. Không dùng loop đếm từng bit một — dùng bitwise trick.

<details>
<summary>Xem đáp án</summary>

**Cách 1 — Brian Kernighan's algorithm:**
```c
int count_bits(uint8_t x) {
    int count = 0;
    while (x) {
        x &= (x - 1);  // xóa bit 1 thấp nhất
        count++;
    }
    return count;
}
```

`x & (x-1)` luôn xóa bit `1` thấp nhất. Số lần lặp = số bit `1`.

**Cách 2 — lookup table (nhanh hơn cho embedded):**
```c
static const uint8_t lut[16] = {0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4};

int count_bits(uint8_t x) {
    return lut[x & 0x0F] + lut[x >> 4];
}
```

</details>

---

**C02.** Viết hàm `is_power_of_two(uint32_t n)` trả về `1` nếu `n` là lũy thừa của 2, `0` nếu không. Không dùng vòng lặp.

<details>
<summary>Xem đáp án</summary>

```c
int is_power_of_two(uint32_t n) {
    return (n != 0) && ((n & (n - 1)) == 0);
}
```

**Lý do:** Số lũy thừa 2 có đúng **1 bit `1`** trong binary. `n & (n-1)` xóa bit `1` thấp nhất → nếu kết quả `0`, chỉ có 1 bit `1` → là lũy thừa 2.

Ví dụ: `n = 8 = 1000`, `n-1 = 7 = 0111`, `8 & 7 = 0` ✅

`n = 0` phải xử lý riêng vì `0 & (0-1) = 0` nhưng 0 không phải lũy thừa 2.

</details>

---

**C03.** Viết hàm đảo ngược bit của `uint8_t`. Ví dụ: `0b10110001` → `0b10001101`.

<details>
<summary>Xem đáp án</summary>

```c
uint8_t reverse_bits(uint8_t x) {
    uint8_t result = 0;
    for (int i = 0; i < 8; i++) {
        result = (result << 1) | (x & 1);
        x >>= 1;
    }
    return result;
}
```

**Hoặc bằng bitwise trick (không loop):**
```c
uint8_t reverse_bits(uint8_t x) {
    x = ((x & 0xF0) >> 4) | ((x & 0x0F) << 4);  // swap nibble
    x = ((x & 0xCC) >> 2) | ((x & 0x33) << 2);  // swap từng cặp 2 bit
    x = ((x & 0xAA) >> 1) | ((x & 0x55) << 1);  // swap từng bit liền kề
    return x;
}
```

</details>

---

**C04.** Viết hàm `swap(int *a, int *b)` không dùng biến tạm, chỉ dùng XOR.

<details>
<summary>Xem đáp án</summary>

```c
void swap(int *a, int *b) {
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}
```

**Cảnh báo quan trọng:** Không dùng khi `a == b` (cùng địa chỉ) → kết quả sai (`*a` thành 0).

**Trong thực tế:** Dùng biến tạm vẫn tốt hơn — compiler tối ưu tốt, không có cạm bẫy alias.

</details>

---

**C05.** Viết hàm `set_bit`, `clear_bit`, `toggle_bit`, `check_bit` cho `uint8_t`.

<details>
<summary>Xem đáp án</summary>

```c
// Set bit thứ n (n từ 0 đến 7)
uint8_t set_bit(uint8_t x, int n)    { return x | (1 << n); }

// Clear bit thứ n
uint8_t clear_bit(uint8_t x, int n)  { return x & ~(1 << n); }

// Toggle bit thứ n
uint8_t toggle_bit(uint8_t x, int n) { return x ^ (1 << n); }

// Kiểm tra bit thứ n (trả về 0 hoặc 1)
int check_bit(uint8_t x, int n)      { return (x >> n) & 1; }
```

**Lưu ý:** `(1 << n)` với `int` literal `1` — nếu `n >= 31` có thể UB. Trong embedded nên dùng `(1U << n)` hoặc `(UINT8_C(1) << n)`.

</details>

---

**C06.** Viết hàm `bubble_sort_desc` sắp xếp mảng theo thứ tự **giảm dần**. Thêm tối ưu early-exit khi mảng đã sort.

<details>
<summary>Xem đáp án</summary>

```c
void bubble_sort_desc(int arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        int swapped = 0;
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j] < arr[j + 1]) {  // đổi dấu so với tăng dần
                int tmp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = tmp;
                swapped = 1;
            }
        }
        if (!swapped) break;  // early exit: đã sort xong
    }
}
```

</details>

---

**C07.** Viết hàm `linear_search_all` tìm **tất cả** vị trí của `target` trong mảng và lưu vào mảng `results`. Trả về số lượng tìm thấy.

<details>
<summary>Xem đáp án</summary>

```c
int linear_search_all(int arr[], int n, int target,
                      int results[], int max_results) {
    int count = 0;
    for (int i = 0; i < n && count < max_results; i++) {
        if (arr[i] == target) {
            results[count++] = i;
        }
    }
    return count;
}
```

**Điểm quan trọng:**
- `count < max_results`: tránh tràn buffer `results`
- Trả về count để caller biết có bao nhiêu kết quả

</details>

---

**C08.** Viết hàm counter theo pattern embedded driver đúng cách — có `reset`, `increment`, `get`. Không để bên ngoài truy cập trực tiếp vào biến.

<details>
<summary>Xem đáp án</summary>

```c
// counter.h
void counter_reset(void);
void counter_increment(void);
int  counter_get(void);

// counter.c
static int s_count = 0;  // static: ẩn khỏi file khác

void counter_reset(void)     { s_count = 0; }
void counter_increment(void) { s_count++; }
int  counter_get(void)       { return s_count; }
```

**Pattern này:**
- `static` ẩn `s_count` khỏi file khác (internal linkage)
- Không trả về pointer → bên ngoài không thể ghi trực tiếp
- Tương đương `private` trong OOP

</details>

---

**C09.** Viết hàm `safe_add_u8(uint8_t a, uint8_t b, uint8_t *result)` trả về `0` nếu thành công, `-1` nếu overflow. Không dùng kiểu lớn hơn để tính.

<details>
<summary>Xem đáp án</summary>

```c
int safe_add_u8(uint8_t a, uint8_t b, uint8_t *result) {
    if (a > 255 - b) {  // nếu a + b > 255 thì overflow
        return -1;
    }
    *result = a + b;
    return 0;
}
```

**Trick:** Thay vì tính `a + b` (có thể overflow), kiểm tra `a > 255 - b` — phép trừ này an toàn vì `255 - b` luôn trong range `[0, 255]`.

</details>

---

**C10.** Viết chương trình hoàn chỉnh tìm phần tử lớn nhất và nhỏ nhất trong mảng bằng **một lần duyệt duy nhất** (không sort, không 2 vòng lặp).

<details>
<summary>Xem đáp án</summary>

```c
#include <stdio.h>

void find_min_max(int arr[], int n, int *min, int *max) {
    if (n <= 0) return;
    *min = *max = arr[0];
    for (int i = 1; i < n; i++) {
        if (arr[i] < *min) *min = arr[i];
        if (arr[i] > *max) *max = arr[i];
    }
}

int main(void) {
    int arr[] = {3, 1, 4, 1, 5, 9, 2, 6};
    int n = sizeof(arr) / sizeof(arr[0]);
    int min, max;

    find_min_max(arr, n, &min, &max);
    printf("min=%d max=%d\n", min, max);  // min=1 max=9
    return 0;
}
```

**O(n) — chỉ 1 lần duyệt**, khởi tạo `min = max = arr[0]` để tránh dùng `INT_MAX`/`INT_MIN`.

</details>

---

**C11.** Viết macro `ARRAY_SIZE(arr)` trả về số phần tử của mảng. Tại sao không dùng `sizeof(arr) / sizeof(arr[0])` trực tiếp?

<details>
<summary>Xem đáp án</summary>

```c
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
```

**Tại sao cần macro?** Khi truyền mảng vào hàm, nó **decay thành pointer** — `sizeof` pointer ≠ `sizeof` mảng:

```c
void foo(int arr[]) {
    // sizeof(arr) = sizeof(int*) = 8 bytes (64-bit)
    // KHÔNG phải kích thước mảng!
}
```

**Macro chỉ đúng khi dùng với mảng thật** (không phải pointer). Trong C11 có thể thêm static assert để bảo vệ:
```c
#define ARRAY_SIZE(arr) \
    (sizeof(arr) / sizeof((arr)[0]))
```

</details>

---

**C12.** Viết hàm `clamp(int val, int min, int max)` giới hạn giá trị trong khoảng `[min, max]`. Đây là hàm cực kỳ phổ biến trong embedded (PWM, ADC scaling...).

<details>
<summary>Xem đáp án</summary>

```c
int clamp(int val, int min_val, int max_val) {
    if (val < min_val) return min_val;
    if (val > max_val) return max_val;
    return val;
}
```

**Hoặc dùng ternary:**
```c
int clamp(int val, int min_val, int max_val) {
    return val < min_val ? min_val :
           val > max_val ? max_val : val;
}
```

**Lưu ý:** Không dùng tên `min`/`max` vì có thể conflict với macro trong `<sys/param.h>`.

</details>

---

**C13.** Viết chương trình đọc mảng số nguyên, in ra các số chia hết cho 3 nhưng không chia hết cho 5, dùng `continue` hợp lý.

<details>
<summary>Xem đáp án</summary>

```c
#include <stdio.h>

int main(void) {
    int arr[] = {1, 3, 5, 6, 9, 10, 12, 15, 18, 20, 21};
    int n = sizeof(arr) / sizeof(arr[0]);

    for (int i = 0; i < n; i++) {
        if (arr[i] % 5 == 0) continue;  // skip bội số 5
        if (arr[i] % 3 != 0) continue;  // skip không chia hết 3
        printf("%d\n", arr[i]);
    }
    return 0;
}
// Output: 3 6 9 12 18 21
```

</details>

---

**C14.** Viết hàm `init_system` mô phỏng embedded initialization với 4 bước. Mỗi bước có thể fail. Dùng `goto` pattern để cleanup đúng cách.

<details>
<summary>Xem đáp án</summary>

```c
#include <stdio.h>

int open_uart(void)    { printf("open uart\n");    return 0; }
int init_spi(void)     { printf("init spi\n");     return -1; } // giả lập fail
int alloc_dma(void)    { printf("alloc dma\n");    return 0; }
int start_timer(void)  { printf("start timer\n");  return 0; }

void close_uart(void)  { printf("close uart\n"); }
void deinit_spi(void)  { printf("deinit spi\n"); }
void free_dma(void)    { printf("free dma\n"); }
void stop_timer(void)  { printf("stop timer\n"); }

int init_system(void) {
    if (open_uart()   < 0) goto err_uart;
    if (init_spi()    < 0) goto err_spi;
    if (alloc_dma()   < 0) goto err_dma;
    if (start_timer() < 0) goto err_timer;
    return 0;

err_timer: free_dma();
err_dma:   deinit_spi();
err_spi:   close_uart();
err_uart:  return -1;
}

int main(void) {
    int r = init_system();
    printf("result = %d\n", r);
    return 0;
}
```

**Output:**
```
open uart
init spi
close uart
result = -1
```

</details>

---

**C15.** Viết hàm `print_binary(uint8_t x)` in ra biểu diễn nhị phân của `x` dạng `0b00001010`.

<details>
<summary>Xem đáp án</summary>

```c
#include <stdio.h>
#include <stdint.h>

void print_binary(uint8_t x) {
    printf("0b");
    for (int i = 7; i >= 0; i--) {
        printf("%d", (x >> i) & 1);
    }
    printf("\n");
}

int main(void) {
    print_binary(10);   // 0b00001010
    print_binary(255);  // 0b11111111
    print_binary(0);    // 0b00000000
    return 0;
}
```

`(x >> i) & 1`: shift bit thứ `i` về bit 0, rồi mask để lấy đúng 1 bit.

</details>

---

**C16.** Viết hàm `count_in_range(int arr[], int n, int lo, int hi)` đếm số phần tử trong khoảng `[lo, hi]` bằng **một lần duyệt**.

<details>
<summary>Xem đáp án</summary>

```c
int count_in_range(int arr[], int n, int lo, int hi) {
    int count = 0;
    for (int i = 0; i < n; i++) {
        if (arr[i] >= lo && arr[i] <= hi) {
            count++;
        }
    }
    return count;
}
```

**O(n)** — một lần duyệt, không sort, không nested loop.

</details>

---

**C17.** Viết hàm `rotate_left(uint8_t x, int n)` rotate left `n` bit (không phải shift — bit tràn ra trái quay về bên phải).

<details>
<summary>Xem đáp án</summary>

```c
uint8_t rotate_left(uint8_t x, int n) {
    n &= 7;  // đảm bảo n trong [0,7]
    return (uint8_t)((x << n) | (x >> (8 - n)));
}
```

**Giải thích:**
- `x << n`: shift trái, bit tràn ra mất
- `x >> (8 - n)`: lấy những bit đã tràn, đặt vào bên phải
- OR lại → rotate

Ví dụ: `rotate_left(0b10110001, 2)` = `0b11000110`

</details>

---

**C18.** Viết chương trình dùng `static` để đếm số lần một function được gọi, không dùng biến global. Function phải thread-safe về mặt interface (chỉ cần đảm bảo đúng logic, không cần actual mutex).

<details>
<summary>Xem đáp án</summary>

```c
#include <stdio.h>

int process(int data) {
    static int call_count = 0;
    call_count++;
    printf("[call #%d] processing %d\n", call_count, data);
    return data * 2;
}

int get_process_count(void) {
    // Không thể truy cập call_count trực tiếp từ đây
    // Cần redesign nếu muốn expose count
    return -1;
}

int main(void) {
    process(10);
    process(20);
    process(30);
    return 0;
}
```

**Output:**
```
[call #1] processing 10
[call #2] processing 20
[call #3] processing 30
```

</details>

---

**C19.** Viết hàm `selection_sort` và so sánh với `bubble_sort` — khi nào selection sort tốt hơn bubble sort trong embedded?

<details>
<summary>Xem đáp án</summary>

```c
void selection_sort(int arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        int min_idx = i;
        for (int j = i + 1; j < n; j++) {
            if (arr[j] < arr[min_idx]) {
                min_idx = j;
            }
        }
        if (min_idx != i) {
            int tmp = arr[min_idx];
            arr[min_idx] = arr[i];
            arr[i] = tmp;
        }
    }
}
```

**So sánh:**

| | Bubble Sort | Selection Sort |
|---|---|---|
| Số phép so sánh | O(n²) | O(n²) |
| Số phép swap | O(n²) worst case | **O(n) luôn luôn** |
| Khi nào tốt hơn | Mảng gần sort (early-exit) | **Swap tốn kém** (flash memory, EEPROM) |

**Trong embedded:** Nếu ghi vào EEPROM (giới hạn số lần ghi), Selection Sort ít swap hơn → tuổi thọ EEPROM lâu hơn.

</details>

---

**C20.** Viết hàm `safe_shift_left(uint32_t x, int n)` thực hiện left shift an toàn — kiểm tra và tránh Undefined Behavior.

<details>
<summary>Xem đáp án</summary>

```c
#include <stdint.h>

uint32_t safe_shift_left(uint32_t x, int n) {
    if (n < 0 || n >= 32) {
        return 0;  // undefined behavior nếu n < 0 hoặc n >= 32
    }
    return x << n;
}
```

**Tại sao `n >= 32` là UB?** Shift amount ≥ số bit của kiểu → C standard nói đây là UB. CPU x86 thực tế dùng `n mod 32`, nhưng ARM có thể cho kết quả khác → không portable.

**Dùng `uint32_t` thay vì `int32_t`** để tránh thêm UB từ signed overflow khi shift trái.

</details>

---

*— Hết GĐ1 Practice Questions —*
ENDOFFILE
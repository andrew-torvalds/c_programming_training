# Log Cuộc Trò Chuyện — Học C Từ Đầu Hướng Embedded Linux
# Session 3 — GĐ2: I/O Functions

---

## Bối cảnh

**Giang:** Tiếp tục từ Session 2 đã hoàn thành GĐ1. Session này bắt đầu GĐ2 — I/O Functions.

**Trạng thái đầu session:**
- GĐ1 hoàn thành toàn bộ ✅
- GĐ2 — I/O Functions: bắt đầu

---

## Phần 1: Format Specifiers

### Kiểm tra điểm xuất phát

**Claude:** Hỏi bạn đang dùng printf/scanf ở mức nào.

**Giang:** Đã biết dùng printf và scanf, `%d` in số nguyên, `%f` số thực, `%s` chuỗi — chỉ biết cơ bản chưa đủ sâu.

**Claude:** Bỏ qua phần cơ bản, đi thẳng vào phần sâu.

---

### `%d` vs `%u` — bit pattern

**Claude:** Hỏi `%d` và `%u` khác nhau thế nào ở mức bit.

**Giang:** `%d` cho số nguyên có dấu, `%u` không dấu.

**Claude:** Đúng. Demo:
```c
int x = -1;
printf("%d\n", x);  // -1
printf("%u\n", x);  // 4294967295
```
printf không quan tâm kiểu biến — chỉ đọc bit pattern theo specifier.

---

### Trap: dùng sai specifier

**Claude:** Hỏi `uint32_t x = 4294967295; printf("%d\n", x)` có vấn đề gì không.

**Giang:** Sẽ có vấn đề, số x sẽ bị giảm lại vì `%d` chỉ in số nguyên có dấu.

**Claude:** Đúng hướng. `%d` đọc `0xFFFFFFFF` như signed → `-294967296`. Rule: dùng đúng specifier cho đúng kiểu.

---

### Width và zero-padding

**Claude:** Giải thích `%02X` và `%08X` — con số 2 và 8 có nghĩa gì.

**Giang:** Số 2 và 8 có nghĩa là thêm số 0 phía trước cho đủ 2 hoặc 8 số in ra màn hình.

**Claude:** Chính xác — width + zero-padding. Phổ biến trong embedded khi debug register.

---

## Phần 2: Float & printf

### `%f` và floating point precision

**Giang:** Từng thấy khi dùng `%f` có số rất nhỏ phần thập phân, và so sánh 2 float bằng nhau thì kết quả không bằng. Nhưng không biết tại sao.

**Claude:** Hỏi: bạn biết CPU lưu số thực như thế nào không?

**Giang:** Không biết.

**Claude:** Giải thích IEEE 754 — float 32-bit chia 3 phần: Sign (1 bit), Exponent (8 bit), Mantissa (23 bit). 23 bit mantissa chỉ biểu diễn được ~8 triệu giá trị → hầu hết số thập phân không chính xác.

**Giang:** Bạn có thể biểu diễn bằng hình ảnh như đã từng làm với signed/unsigned chưa?

**Claude:** Tạo hình SVG inline minh họa cấu trúc IEEE 754 với ví dụ `0.1f = 0.10000000149...`

---

### Float comparison trap

**Claude:** Hỏi `0.1f + 0.2f == 0.3f` — output gì?

**Giang:** Tùy trường hợp — trên VSCode thì bằng, nhưng trên MCU thì không bằng.

**Claude:** Quan sát rất thực tế! Lý do cụ thể: x86 FPU 80-bit tính precision cao hơn → kết quả trung gian chính xác hơn → có thể bằng. MCU FPU 32-bit tính thẳng 32-bit → sai số khác → không bằng.

**Giang:** Chưa hiểu lắm phần này.

**Claude:** Giải thích bằng số cụ thể:
```
MCU 32-bit:
0.1f + 0.2f = 0.300000004470  ≠  0.300000011920 (0.3f) → KHÔNG bằng

x86 80-bit:
tính 80-bit → round về 32-bit → tình cờ bằng 0.3f → BẰNG
```

---

### Epsilon

**Claude:** Rule: không bao giờ dùng `==` với float. Dùng epsilon:
```c
if (fabsf(a - b) < 1e-6f) { ... }
```

**Giang:** Chưa hiểu về khái niệm epsilon và số e trong C.

**Claude:** Giải thích:
- `1e-6f` = `1 × 10⁻⁶ = 0.000001` (scientific notation)
- `f` ở cuối = float literal
- Epsilon = ngưỡng sai số chấp nhận được, không phải keyword của C

---

### Float literal và hậu tố `f`

**Giang:** `1e-6` đã được rồi vì sao phải thêm `f`?

**Claude:** Hỏi: `3.14` mặc định là kiểu gì?

**Giang:** Mặc định là float.

**Claude:** Ngược lại — `3.14` mặc định là **double**. Mọi float literal không có `f` → `double`.

**Giang:** Nếu dùng `1e-6` (double) so sánh với float thì có vấn đề gì?

**Claude:** Float bị convert lên double (implicit conversion lên kiểu chính xác hơn). Trên MCU không có double FPU → software emulation → chậm hơn 10-100 lần. Rule: luôn thêm `f` vào float literal trong embedded.

---

### `0.1f` vs `0.1` — cái nào lớn hơn?

**Giang:** Vậy so sánh `0.1f` với `0.1` thì số nào lớn hơn?

**Claude:** Hỏi lại để bạn suy luận.

**Giang:** Mình nghĩ `0.1f`.

**Claude:** Chính xác — `0.1f` có ít bit hơn → round lên → lớn hơn `0.1` double. Nhưng không phải float luôn lớn hơn double — tùy giá trị.

---

## Phần 3: scanf

### Tại sao scanf cần `&`

**Claude:** Hỏi tại sao `scanf` cần `&x` còn `printf` chỉ cần `x`.

**Giang:** `&` trong C là cách lấy địa chỉ của biến.

**Claude:** Đúng, nhưng tại sao scanf cần địa chỉ còn printf thì không?

**Giang:** scanf cần địa chỉ để tác động trực tiếp và thay đổi giá trị, printf chỉ cần đọc không cần sửa.

**Claude:** Chính xác hoàn toàn.

---

### Buffer overflow với `%s`

**Claude:** Hỏi `char name[8]; scanf("%s", name);` nhập "Nguyen Van An" có vấn đề gì.

**Giang:** scanf khi gặp enter sẽ dừng lại (không đọc được khoảng trắng).

**Claude:** Đúng một phần — `scanf("%s")` dừng khi gặp whitespace, không chỉ Enter. Nhưng có vấn đề nguy hiểm hơn: nếu nhập chuỗi dài hơn buffer → buffer overflow.

**Giang:** Nó sẽ ghi nối tiếp từ địa chỉ `name[8]`, nguy hiểm vì địa chỉ đó có thể là giá trị của biến khác.

**Claude:** Chính xác — buffer overflow, ghi đè stack, crash hoặc security vulnerability.

---

### `fgets` thay vì `scanf`

**Claude:** Fix dùng `fgets`:
```c
fgets(name, sizeof(name), stdin);
```

**Giang:** `fgets` có thể nhận ký tự khoảng trắng, nhưng khi gặp Enter thì đưa vào buffer và kết thúc.

**Claude:** Chính xác. Và `\n` được lưu vào buffer — trap hay gặp khi dùng `strcmp`. Fix:
```c
name[strcspn(name, "\n")] = '\0';
```

**Giang:** Thực tế `fgets` dùng trong trường hợp nào?

**Claude:** Hỏi lại bằng Why method.

**Giang:** Khi nhận giá trị có space từ keyboard.

**Claude:** Đúng. Bổ sung thêm: đọc file config, nhận lệnh UART, parse CSV. `scanf` gần như không dùng trong embedded production code.

---

## Phần 4: stdin / stdout / stderr

### 3 stream mặc định

**Giang:** Đã từng nghe nhưng chưa biết gì hết.

**Claude:** Giải thích: OS tự động mở 3 stream khi chương trình khởi động — stdin (input), stdout (output, line buffered), stderr (error, unbuffered).

---

### Tại sao cần 2 stream riêng?

**Giang:** Mình chỉ hiểu bề nổi là phân biệt nhưng chưa hiểu ý nghĩa sâu xa.

**Claude:** Giải thích bằng tình huống thực tế — chạy firmware test 8 tiếng, redirect độc lập:
```bash
./sensor_program > sensor_log.txt 2> error_log.txt
```
`error_log.txt` chỉ có lỗi → dễ tìm ngay. Dùng trong CI/CD pipeline để tự động detect failure.

**Giang:** À hiểu.

---

### Buffer khác nhau

**Claude:** Hỏi bạn có nhớ stdout và stderr buffer khác nhau thế nào không.

**Giang:** Mình nhớ bạn chưa từng nói về vấn đề này.

**Claude:** Bạn nhớ đúng — đã đề cập thoáng qua ở GĐ1.1 nhưng chưa giải thích kỹ. Giải thích:
- `stdout`: line buffered — flush khi `\n`, buffer đầy, `fflush()`, exit
- `stderr`: unbuffered — ghi ra ngay lập tức

Lý do: lỗi cần hiện ngay — nếu crash trước khi flush, `stdout` mất output, `stderr` không mất.

---

### File I/O

**Giang:** Thực tế `fopen`, `fread`... có quan trọng trong embedded không? Mình chưa thấy ai dùng.

**Claude:** Trên bare-metal MCU: gần như không dùng. Trên Embedded Linux: có dùng — đọc `/sys/class/gpio`, file config, log ra SD card.

**Giang:** Mình đã học file I/O trong Linux programming course rồi, sẽ bỏ qua.

**Claude:** Hợp lý — tránh học trùng lặp.

---

## Phần 5: Tổng hợp tài liệu

### Tạo GD2_C_Lesson_Notes.md + SVG

**Giang:** Muốn tổng hợp tài liệu GĐ2, có hình ảnh SVG để chèn vào file .md.

**Claude:** Tạo 2 file:
- `GD2_C_Lesson_Notes.md` — tài liệu đầy đủ có mục lục + emoji
- `GD2_ieee754_float.svg` — hình minh họa cấu trúc IEEE 754

### Tạo GD2_Practice_Questions.md

**Giang:** Làm câu hỏi ôn tập như GĐ1.

**Claude:** Tạo file 60 câu, 3 phần:
- Phần A — Lý thuyết: 20 câu
- Phần B — Đọc code: 25 câu
- Phần C — Viết code: 15 câu

**Giang:** Upload file GĐ1 để đối chiếu format — format đúng rồi, ổn.

---

## Trạng thái cuối session

**GĐ2 đã hoàn thành:**
- 2.1 Format Specifiers ✅
- 2.2 Float & printf (IEEE 754, epsilon, float literal) ✅
- 2.3 scanf (buffer overflow, fgets, \n trap) ✅
- 2.4 stdin/stdout/stderr ✅
- File I/O — bỏ qua (đã học ở Linux programming course)

**Files đã tạo trong session này:**
- `GD2_C_Lesson_Notes.md` — tài liệu học có mục lục + emoji
- `GD2_ieee754_float.svg` — hình IEEE 754
- `GD2_Practice_Questions.md` — 60 câu hỏi collapsible (2317 dòng)

**Tiếp theo:** GĐ3 — Memory Model (stack/heap/data/BSS sâu, dynamic allocation, memory bugs)

---

## Kiến thức trọng tâm GĐ2

| Chủ đề | Điểm cốt lõi |
|---|---|
| Format specifier | printf đọc bit pattern theo specifier — dùng sai → kết quả sai |
| `%zu` | Specifier đúng cho `size_t` — không dùng `%d` |
| IEEE 754 | 23 bit mantissa → không thể biểu diễn chính xác phần lớn số thập phân |
| Float comparison | Không dùng `==` — dùng `fabsf(a-b) < epsilon` |
| Float literal | Mọi số thực literal mặc định là `double` — phải thêm `f` trong embedded |
| `scanf` buffer overflow | `%s` không giới hạn độ dài — dùng `fgets` thay thế |
| `fgets` + `\n` | `fgets` lưu `\n` vào buffer — phải xóa bằng `strcspn` |
| stderr unbuffered | Dùng `fprintf(stderr,...)` cho lỗi — không mất khi crash |
| Redirect stream | `> file` cho stdout, `2> file` cho stderr — redirect độc lập |

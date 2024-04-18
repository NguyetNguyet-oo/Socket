# Socket
Lập trình socket, xây dựng giao thức trao đổi giữa client và server.
Nội dung:
Mục tiêu là viết một web server đơn giản. Webserver sẽ bind trên port 8080. Bạn có thể dùng bất kỳ trình duyệt web nào làm client (Firefox, Chrome, Safari, ...) để gửi request đến web server và webserver sẽ gửi lại response cho client (nội dung của page, hoặc là lỗi). Nội dung của trang web đã được lập trình sẵn, web server sẽ đọc nội dung file và trả về cho client

1. Handle connection cơ bản:
Server tạo một TCP socket, bind trên port 8080, sau đó listen và accept connection từ browser. Khi chấp nhận kết nối từ browser, server đọc và parse request. Dòng đầu tiên của data (request line) sẽ kết thúc bằng (\r\n) để xác định được kiểu của HTTP request tạo bởi browser.
- Nếu request bắt đầu với "GET" và theo sau đó là "/" hoặc tên filename ( ví dụ index.html, css/style.css, ...), cuối cùng là HTTP/1.1, server cần phải parse thông tin của request để biết được cần phải load file nào cho request đó. Nếu là "/" thì coi như "index.html". Tên filename sẽ liên quan đến thư mục của máy chủ tương ứng (i.e. css/style.css, avatars/1.png, ...). Sau khi parse request để xác định filename, server sẽ gửi reponse với HTTP header tương ứng, kèm theo là body (nội dung của file). Sau đó đóng kết nối (không giữ connection - không Keep alive)
  - Nếu filename tồn tại, thì trả về thành công "200 OK" kèm theo nội dung của file.
  - Nếu file không tồn tại, trả về "404 File Not Found". Đây là một ví dụ của một response "404 File Not Found"
- Nếu request bắt đầu với "POST", thì server sẽ nhận được thông tin "uname" và "psw" kèm theo trong body của request. Server cần parse thông tin này ra và kiểm tra nếu "uname" là "admin" và "psw" là "123456" thì trả về nội dung của trang "images.html". Nếu không thì trả về "401 Unauthorized". 
2. Nâng cao: 
- Cho phép gửi nhiều request cho mỗi kết nối: Hiện tại web browser mở một TCP connection cho mỗi request khi nhiều file cần được truy cập => ko hiệu quả, HTTP cho phép gửi nhiều request trên một kết nối trước đó. Nếu server không có header "Connection: close" trong response, client có thể giữ kết nối, và tiếp tục gửi những request tiếp theo. Để cho phép Client có thể phân biệt data từ nhiều request, server bắt buộc phải có "Content-Length:" header để chỉ định kích thước của data trả về cho client. Kết nối chỉ đóng lại khi client đóng kết nối, khi đó hàm "recv" trên server sẽ return 0 khi kết nối bị đóng.
- Cho phép nhiều clients truy cập cùng lúc (Handling Multiple Connections in Parallel, concurrency by thread for each connection, ...)

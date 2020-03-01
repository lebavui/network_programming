# network_programming_tutorial
Các ví dụ network programming

1. SimpleClient
	+ Ứng dụng client đơn giản kết nối server được tạo bởi netcat
	+ Client nhận dữ liệu từ server và in ra màn hình console
2. SimpleHTTPClient
	+ Ứng dụng http client, kết nối đến website
	+ Thực hiện phân giải tên miền
	+ Gửi lệnh GET và hiển thị kết quả trả về
3. SimpleServer
	+ Ứng dụng server đơn giản chấp nhận 1 client
	+ Server đọc dữ liệu từ bàn phím và gửi cho client
4. NonBlockingServer
	+ Ứng dụng server sử dụng socket ở chế độ non-blocking
	+ Các hàm vào ra accept(), recv() trả về kết quả ngay lập tức
	+ Chương trình cần kiểm tra kết quả trả về của các hàm này, nếu bị lỗi mà lỗi do đang chờ thao tác vào ra thì bỏ qua

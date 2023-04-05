#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>

/// @brief Hàm xóa client khỏi mảng
/// @param clients Mảng client đang kết nối đến server
/// @param pNumClients Địa chỉ biến chứa số lượng client
/// @param index Thứ tự của phần tử cần xóa
void removeClient(int *clients, int *pNumClients, int index)
{
    if (index < *pNumClients - 1)
        clients[index] = clients[*pNumClients - 1];
    *pNumClients = *pNumClients - 1;
}

int main() 
{
    // Tạo socket
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    // Chuyển socket sang trạng thái bất đồng bộ
    unsigned long ul = 1;
    ioctl(listener, FIONBIO, &ul);

    // Khai báo cấu trúc địa chỉ server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    // Gắn địa chỉ với socket
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) 
    {
        perror("bind() failed");
        return 1;
    }

    if (listen(listener, 5)) 
    {
        perror("listen() failed");
        return 1;
    }

    int clients[64];
    int numClients = 0;
    char buf[256];
    
    while (1)
    {
        // Chấp nhận kết nối
        int client = accept(listener, NULL, NULL);
        if (client == -1)
        {
            // Nếu lỗi không phải do đang chờ kết nối
            if (errno != EWOULDBLOCK)
            {
                perror("accept() failed");
                return 1;
            }
            else
            {
                // Nếu lỗi do đang chờ kết nối thì bỏ qua, thực hiện công việc khác
            }
        }
        else
        {
            // Nếu có kết nối mới thì thêm vào mảng và chuyển sang trạng thái bất đồng bộ
            printf("New client connected: %d\n", client);
            clients[numClients++] = client;
            ul = 1;
            ioctl(client, FIONBIO, &ul);
        }
        
        // Kiểm tra các client có truyền dữ liệu không
        for (int i = 0; i < numClients; i++)
        {
            int ret = recv(clients[i], buf, sizeof(buf), 0);
            if (ret == -1)
            {
                if (errno != EWOULDBLOCK)
                {
                    // Nếu lỗi không phải do đang chờ dữ liệu
                    // Xóa client khỏi mảng
                    // Chuyển sang kiểm tra kết nối khác
                    perror("recv() failed");
                    close(clients[i]);
                    removeClient(clients, &numClients, i--);
                    continue;
                }
                else
                {
                    // Nếu lỗi do đang chờ dữ liệu thì bỏ qua
                }
            }
            else if (ret == 0)
            {
                // Nếu kế nối bị đóng, thì xóa client khỏi mảng
                printf("client disconnected.\n");
                close(clients[i]);
                removeClient(clients, &numClients, i--);
                continue;
            }
            else
            {
                // Xử lý dữ liệu nhận được
                // Thêm ký tự kết thúc xâu và in ra màn hình
                if (ret < sizeof(buf))
                    buf[ret] = 0;
                puts(buf);
                // Trả lại kết quả cho client
                send(clients[i], buf, strlen(buf), 0);
            }
        }
    }
    
    close(listener);    

    return 0;
}
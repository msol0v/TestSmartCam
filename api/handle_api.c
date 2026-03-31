//
// Created by msol0v on 30.03.2026.
//

#include "handle_api.h"

#include "fs.h"

#include "../motor/motor.h"

/**
 * Вызывается при получении POST-запроса (начало)
 */

// err_t httpd_post_begin(void *connection, const char *uri, const char *http_request,
//                        u16_t http_request_len, int content_len, char *response_uri,
//                        u16_t response_uri_len, u8_t *post_auto_wnd) {
//
//     // Включаем автоматическое управление окном приема TCP
//     if (post_auto_wnd) {
//         *post_auto_wnd = 1;
//     }
//
//     // Проверяем URI
//     if (strcmp(uri, "/api/plus") == 0) {
//         action = PLUS;
//         return ERR_OK; // Разрешаем POST на этот адрес
//     }
//
//     if (strcmp(uri, "/api/minus") == 0) {
//         action = MINUS;
//         return ERR_OK; // Разрешаем POST на этот адрес
//     }
//
//     return ERR_VAL; // Отклоняем все остальные POST запросы
// }

/**
 * Вызывается при получении фрагментов данных тела POST
 */
// err_t httpd_post_receive_data(void *connection, struct pbuf *p) {
//     if (p != NULL) {
//         // Здесь можно обработать данные из p->payload
//         // ВАЖНО: Мы не храним pbuf, lwIP сам его удалит после возврата ERR_OK
//         pbuf_free(p);
//     }
//     return ERR_OK;
// }

/**
 * Вызывается, когда весь POST запрос получен полностью
 */
// void httpd_post_finished(void *connection, char *response_uri, u16_t response_uri_len) {
//     // Указываем страницу, которую сервер отдаст в ответ (редирект или успех)
//     //snprintf(response_uri, response_uri_len, "/success_json.html");
//     if (action == PLUS) {
//         printf("httpd_post_PLUS\r\n");
//     }
//     else if (action == MINUS) {
//         printf("httpd_post_MINUS\r\n");
//     }
//     action = NONE_ACTION;
// }
//
// const tCGI api_handlers[] = {
//     {"/api/plus", plus_cgi_handler},
//     {"/api/minus", minus_cgi_handler},
// };
//
// const char* plus_cgi_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]) {
//     // Логика управления
//     return "/index.html"; // Куда перенаправить пользователя
// }
//
// const char* minus_cgi_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]) {
//     // Логика управления
//     return "/index.html"; // Куда перенаправить пользователя
// }
static unsigned int count = 0;
static char json_buf[512]; // Буфер должен быть static и достаточного размера

int fs_open_custom(struct fs_file *file, const char *name) {
    int payload_len = 0;
    int total_len = 0;

    // Очищаем буфер перед использованием
    memset(json_buf, 0, sizeof(json_buf));

    // --- 1. ОБРАБОТКА КОМАНД МОТОРА ---
    // Ожидаем: /api/move?m=1&s=500&d=cw
    if (strstr(name, "api/move")) {
        MotorCommand_t cmd;
        int m_id_tmp = 0;
        int32_t s_val_tmp = 0;

        // Парсим ID мотора и количество шагов
        char *m_ptr = strstr(name, "m=");
        char *s_ptr = strstr(name, "s=");

        if (m_ptr && s_ptr) {
            if (sscanf(m_ptr, "m=%d", &m_id_tmp) == 1 &&
                sscanf(s_ptr, "s=%" SCNd32, &s_val_tmp) == 1) {

                cmd.motor_id = (uint8_t)m_id_tmp;
                cmd.steps = s_val_tmp;
                cmd.dir = strstr(name, "d=cw") ? MOTOR_DIR_CW : MOTOR_DIR_CCW;

                // Отправляем в очередь FreeRTOS (без ожидания, чтобы не вешать стек lwIP)
                osMessageQueuePut(motorQueueHandle, &cmd, 0, 0);

            }
        }

        // Подготовка JSON тела
        payload_len = snprintf(NULL, 0, "{\"status\":\"queued\",\"motor\":%d}", m_id_tmp);

        // Сборка полного пакета: Заголовки + Тело
        total_len = snprintf(json_buf, sizeof(json_buf),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: %d\r\n"
            "Cache-Control: no-cache, no-store, must-revalidate\r\n"
            "Connection: close\r\n"
            "\r\n"
            "{\"status\":\"queued\",\"motor\":%d}",
            payload_len, m_id_tmp);
    }

    // --- 2. ОБРАБОТКА СЧЕТЧИКА (System Check) ---
    else if (strstr(name, "api/plus") || strstr(name, "api/minus") || strstr(name, "api/value")) {
        if (strstr(name, "api/plus")) count++;
        else if (strstr(name, "api/minus")) count--;

        payload_len = snprintf(NULL, 0, "{\"value\": %u}", (unsigned int)count);

        total_len = snprintf(json_buf, sizeof(json_buf),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: %d\r\n"
            "Cache-Control: no-cache, no-store, must-revalidate\r\n"
            "Connection: close\r\n"
            "\r\n"
            "{\"value\": %u}",
            payload_len, (unsigned int)count);
    }

    // --- 3. ЕСЛИ ЭТО НЕ API ЗАПРОС ---
    else {
        return 0; // lwIP будет искать файл в статическом образе fsdata.c
    }

    // Заполнение структуры ответа для lwIP
    file->data = json_buf;
    file->len = total_len;
    file->index = total_len; // Важно: устанавливаем индекс в конец, чтобы избежать Assert
    file->pextension = NULL;
    file->flags = FS_FILE_FLAGS_HEADER_INCLUDED; // Мы сами сформировали HTTP заголовки

    return 1;
}

void fs_close_custom(struct fs_file *file) {
    // Оставляем пустым
}

// const char *ssi_tags[] = {
//     "c",  // Индекс 0
// };
//
// u16_t ssi_handler(int iIndex, char *pcInsert, int iInsertLen) {
//     if (iIndex == 0) {
//         return (u16_t)snprintf(pcInsert, iInsertLen, "%d", count);
//     }
//     return 0;
// }
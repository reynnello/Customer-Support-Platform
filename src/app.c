#include "app.h"

void app_init(AppState *app)
{
    *app = (AppState){0};
    app->next_id = 1;
}

const char *ticket_status_name(TicketStatus status)
{
    switch (status) {
        case TICKET_OPEN: return "Open";
        case TICKET_IN_PROGRESS: return "In progress";
        case TICKET_RESOLVED: return "Resolved";
        default: return "Unknown";
    }
}

const char *ticket_priority_name(TicketPriority priority)
{
    switch (priority) {
        case PRIORITY_LOW: return "Low";
        case PRIORITY_MEDIUM: return "Medium";
        case PRIORITY_HIGH: return "High";
        default: return "Unknown";
    }
}

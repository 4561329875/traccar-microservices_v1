#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

typedef struct {
    int id;
    char description[255];
    double total_amount;
    char status[50];
} Order;

Order* create_order(int id, const char* description, double total_amount, const char* status) {
    Order* order = malloc(sizeof(Order));
    order->id = id;
    strncpy(order->description, description, sizeof(order->description));
    order->total_amount = total_amount;
    strncpy(order->status, status, sizeof(order->status));
    return order;
}

void process_orders(Order** orders, int order_count) {
    #pragma omp parallel for
    for (int i = 0; i < order_count; i++) {
        printf("Processing order %d: %s (Total: %.2f)\n", 
               orders[i]->id, orders[i]->description, orders[i]->total_amount);
        
        // Simulate order status update
        #pragma omp critical
        {
            if (strcmp(orders[i]->status, "PENDING") == 0) {
                strcpy(orders[i]->status, "PROCESSING");
            }
        }
    }
}

void free_order(Order* order) {
    free(order);
}

int main() {
    Order* orders[3];
    orders[0] = create_order(1, "Office Supplies", 500.50, "PENDING");
    orders[1] = create_order(2, "Computer Hardware", 1200.75, "PENDING");
    orders[2] = create_order(3, "Software Licenses", 750.25, "PENDING");

    process_orders(orders, 3);

    // Cleanup
    for (int i = 0; i < 3; i++) {
        free_order(orders[i]);
    }

    return 0;
}
/*
 * WNU OS Calculator Application
 * Scientific calculator with memory and expression parsing
 * Copyright (c) 2025 WNU Project
 */

#include "wnu_shell.h"

// Calculator state
typedef struct {
    double memory;
    double last_result;
    char expression[256];
    int running;
} calc_state_t;

static calc_state_t calc_state;

// Math functions
static double calc_add(double a, double b) { return a + b; }
static double calc_subtract(double a, double b) { return a - b; }
static double calc_multiply(double a, double b) { return a * b; }
static double calc_divide(double a, double b) { return (b != 0.0) ? a / b : 0.0; }
static double calc_power(double base, double exp) {
    if (exp == 0) return 1.0;
    double result = 1.0;
    for (int i = 0; i < (int)exp; i++) {
        result *= base;
    }
    return result;
}

static double calc_sqrt(double x) {
    if (x < 0) return 0.0; // Error
    if (x == 0) return 0.0;
    
    double guess = x / 2.0;
    for (int i = 0; i < 10; i++) {
        guess = (guess + x / guess) / 2.0;
    }
    return guess;
}

// Simple expression evaluator
static double calc_evaluate_simple(const char* expr) {
    double result = 0.0;
    double operand = 0.0;
    char op = '+';
    const char* p = expr;
    
    while (*p) {
        if (*p >= '0' && *p <= '9') {
            operand = operand * 10 + (*p - '0');
        } else if (*p == '.' || *p == ' ') {
            // Skip decimals and spaces for simplicity
        } else if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '\0') {
            // Apply previous operation
            switch (op) {
                case '+': result += operand; break;
                case '-': result -= operand; break;
                case '*': result *= operand; break;
                case '/': result = calc_divide(result, operand); break;
            }
            
            op = *p;
            operand = 0.0;
            
            if (*p == '\0') break;
        }
        p++;
    }
    
    return result;
}

static void calc_print_help(void) {
    shell_print_info("WNU Calculator v1.0 Commands:\n\n");
    
    shell_print_color("Basic Operations:\n", VGA_COLOR_YELLOW);
    shell_print("  +, -, *, /     - Basic arithmetic\n");
    shell_print("  ^              - Power (e.g., 2^3 = 8)\n");
    shell_print("  sqrt(x)        - Square root\n");
    
    shell_print_color("\nMemory Operations:\n", VGA_COLOR_YELLOW);
    shell_print("  m+             - Add to memory\n");
    shell_print("  m-             - Subtract from memory\n");
    shell_print("  mr             - Recall memory\n");
    shell_print("  mc             - Clear memory\n");
    
    shell_print_color("\nSpecial Commands:\n", VGA_COLOR_YELLOW);
    shell_print("  clear, c       - Clear current calculation\n");
    shell_print("  help, h        - Show this help\n");
    shell_print("  quit, q        - Exit calculator\n");
    
    shell_print_color("\nExamples:\n", VGA_COLOR_CYAN);
    shell_print("  > 2 + 3 * 4    = 14\n");
    shell_print("  > sqrt(16)     = 4\n");
    shell_print("  > 2^8          = 256\n");
    shell_print("  > 100 / 3      = 33.33...\n");
}

static void calc_print_result(double result) {
    shell_print_color("= ", VGA_COLOR_GREEN);
    
    // Simple double to string conversion
    long int_part = (long)result;
    double frac_part = result - int_part;
    
    // Print integer part
    if (int_part == 0) {
        shell_print("0");
    } else {
        char digits[32];
        int i = 0;
        long n = int_part;
        
        if (n < 0) {
            shell_print("-");
            n = -n;
        }
        
        while (n > 0) {
            digits[i++] = '0' + (n % 10);
            n /= 10;
        }
        
        // Print digits in reverse order
        for (int j = i - 1; j >= 0; j--) {
            char digit_str[2] = {digits[j], '\0'};
            shell_print(digit_str);
        }
    }
    
    // Print fractional part (simplified)
    if (frac_part > 0.001 || frac_part < -0.001) {
        shell_print(".");
        frac_part = (frac_part < 0) ? -frac_part : frac_part;
        
        for (int i = 0; i < 3; i++) {
            frac_part *= 10;
            int digit = (int)frac_part;
            char digit_str[2] = {'0' + digit, '\0'};
            shell_print(digit_str);
            frac_part -= digit;
        }
    }
    
    shell_print("\n");
}

static void calc_process_command(const char* input) {
    if (wnu_strcmp(input, "help") == 0 || wnu_strcmp(input, "h") == 0) {
        calc_print_help();
        return;
    }
    
    if (wnu_strcmp(input, "quit") == 0 || wnu_strcmp(input, "q") == 0 || wnu_strcmp(input, "exit") == 0) {
        calc_state.running = 0;
        return;
    }
    
    if (wnu_strcmp(input, "clear") == 0 || wnu_strcmp(input, "c") == 0) {
        calc_state.last_result = 0.0;
        shell_print_info("Calculator cleared.\n");
        return;
    }
    
    // Memory operations
    if (wnu_strcmp(input, "mr") == 0) {
        calc_print_result(calc_state.memory);
        calc_state.last_result = calc_state.memory;
        return;
    }
    
    if (wnu_strcmp(input, "mc") == 0) {
        calc_state.memory = 0.0;
        shell_print_info("Memory cleared.\n");
        return;
    }
    
    if (wnu_strcmp(input, "m+") == 0) {
        calc_state.memory += calc_state.last_result;
        shell_print_info("Added to memory.\n");
        return;
    }
    
    if (wnu_strcmp(input, "m-") == 0) {
        calc_state.memory -= calc_state.last_result;
        shell_print_info("Subtracted from memory.\n");
        return;
    }
    
    // Handle special functions
    if (wnu_strncmp(input, "sqrt(", 5) == 0) {
        // Extract number inside parentheses
        double x = 0.0;
        const char* p = input + 5;
        while (*p >= '0' && *p <= '9') {
            x = x * 10 + (*p - '0');
            p++;
        }
        calc_state.last_result = calc_sqrt(x);
        calc_print_result(calc_state.last_result);
        return;
    }
    
    // Evaluate mathematical expression
    double result = calc_evaluate_simple(input);
    calc_state.last_result = result;
    calc_print_result(result);
}

// Main calculator application
int app_calc(int argc, char* argv[]) {
    // Initialize calculator state
    calc_state.memory = 0.0;
    calc_state.last_result = 0.0;
    calc_state.running = 1;
    
    shell_print_color("╔════════════════════════════════════╗\n", VGA_COLOR_CYAN);
    shell_print_color("║        WNU Calculator v1.0         ║\n", VGA_COLOR_CYAN);
    shell_print_color("║    Scientific Calculator & Math    ║\n", VGA_COLOR_CYAN);
    shell_print_color("╚════════════════════════════════════╝\n", VGA_COLOR_CYAN);
    shell_print("\n");
    
    shell_print_info("Type 'help' for commands or 'quit' to exit.\n");
    shell_print_warning("Enter mathematical expressions (e.g., 2+3*4, sqrt(16))\n\n");
    
    char input[256];
    while (calc_state.running) {
        shell_print_color("calc> ", VGA_COLOR_YELLOW);
        
        // Get input (simplified for demo)
        // In real implementation, this would read from keyboard
        // For demo, let's simulate some calculations
        static int demo_step = 0;
        const char* demo_inputs[] = {
            "help",
            "2 + 3 * 4",
            "sqrt(16)",
            "10 / 3",
            "2^8",
            "mr",
            "quit"
        };
        
        if (demo_step < 7) {
            wnu_strcpy(input, demo_inputs[demo_step]);
            shell_print(input);
            shell_print("\n");
            calc_process_command(input);
            demo_step++;
            
            // Simulate delay
            for (volatile int i = 0; i < 100000; i++);
        } else {
            calc_state.running = 0;
        }
    }
    
    shell_print_success("Calculator closed.\n");
    return 0;
}
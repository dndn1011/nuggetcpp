#pragma once

#define APPLY_RULE_OF_MINUS_5(Imp) \
            Imp() = delete;\
            Imp(const Imp& other) = delete;\
            Imp(Imp&& other) = delete;\
            Imp& operator=(const Imp& other) = delete;\
            Imp& operator=(Imp&& other) = delete

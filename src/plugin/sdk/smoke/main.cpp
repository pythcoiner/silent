#include <sdk/types.h>

int main() {
    plugin::Coin coin{
        .outpoint = "", .value = 0, .height = 0, .label = "", .spent = false, .accountType = ""};
    plugin::Balance balance{.confirmed = 0, .unconfirmed = 0};
    (void)coin;
    (void)balance;
    return 0;
}

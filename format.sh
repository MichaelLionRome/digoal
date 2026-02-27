find . -path ./lib -prune -o -regex '.*\.\(c\|h\|cpp\|hpp\)' -exec clang-format -style=file -i {} \;

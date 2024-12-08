
global array init optimization

```ASM

arr:
	.4byte	1
	.4byte	2
	.4byte	3
	.4byte	0
	.4byte	0
	.4byte	0
	.4byte	0
	.4byte	0
	.4byte	0
	.4byte	0
	.4byte	0
	.4byte	0
	.4byte	0
	.4byte	0
	.4byte	0
	.4byte	0
	.4byte	0
	.4byte	0
	.4byte	0
	.4byte	0
	.4byte	5
	.4byte	6

->

	.globl	arr
	.align	3
	.type	arr, @object
	.size	arr, 400
arr:
	.word	1
	.word	2
	.word	3
	.zero	28
	.zero	40
	.word	5
	.word	6
	.word	7
	.zero	28
	.zero	280
```

```C
int g = 5;

int main() {
    return g;
}
```

```llvm
@g = global i32 5

define i32 @main() {
  %2 = load i32, i32* @g
  ret i32 %2
}
```

```LLVM
@a = dso_local global i32 0, align 4

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @func(i32 noundef %0) #0 {
  %2 = alloca i32, align 4
  store i32 %0, i32* %2, align 4
  %3 = load i32, i32* %2, align 4
  %4 = sub nsw i32 %3, 1
  store i32 %4, i32* %2, align 4
  %5 = load i32, i32* %2, align 4
  ret i32 %5
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  store i32 10, i32* @a, align 4
  %3 = load i32, i32* @a, align 4
  %4 = call i32 @func(i32 noundef %3)
  store i32 %4, i32* %2, align 4
  %5 = load i32, i32* %2, align 4
  ret i32 %5
}

```
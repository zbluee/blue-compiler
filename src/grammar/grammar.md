####Grammar \( G = (N, T, P, S) \) is defined as follows:

- **Non-terminals (\( N \)):**
  - `exit`
  - `expr`
  - `int_literals`
<br>
- **Terminals (\( T \)):**
  - `0, 1, 2, ..., 9`
<br>
- **Start Symbol (S):** 
    - `{exit}`
<br>
- **Production Rules (\( P \)):**

  ```
  [exit] ⟶ exit([expr]);
  [expr] ⟶ int_literals
  [int_literals] ⟶ 0, 1, 3, ..., 9 
  

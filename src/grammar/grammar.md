####Grammar \( G = (N, T, P, S) \) is defined as follows:

- **Non-terminals (\( N \)):**
  - `prog`
  - `statment`
  - `exit`
  - `expr`
  - `int_literals`

- **Terminals (\( T \)):**
  - `0, 1, 2, ..., 9`

- **Start Symbol (S):** 
    - `{prog}`

- **Production Rules (\( P \)):**

  ```
  [prog] ⟶ statment*
  [statment] ⟶  { exit([expr]);
                { let ident = [expr];
  [expr] ⟶  { int_literals
            { ident
  [int_literals] ⟶ 0, 1, 3, ..., 9 
  

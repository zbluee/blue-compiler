####Grammar \( G = (N, T, P, S) \) is defined as follows:

- **Non-terminals (\( N \)):**
  - `Prog`
  - `Statment`
  - `Ident`
  - `letter`
  - `Int_literals`
  - `Expr`
  - `Term`
  - `Exprs`

- **Terminals (\( T \)):**
  - `0, 1, 2, ..., 9`
  - `a, b, c, ... z, A, B, C, ... Z`
  - `exit`
  - `let`
  - `;`
  - `=`
  - `+`
  - `-`
  - `*`
  - `/`
  - `(`
  - `)`

- **Start Symbol (S):** 
    - `Prog`

- **Production Rules (\( P \)):**

  ```
  <Prog> ⟶ <Statment> | <Prog> <Statment>
  
  <Statment> ⟶  exit(<Expr>); | <let> <Ident> = <Expr>;

  <Ident> ⟶  <letter> | <Ident> <letter> | <Ident> <Int_literals>

  <Expr> ⟶  <Term> | <Exprs>

  <Term> ⟶  <Int_literals> | <Ident> | (<Expr>)

  <Exprs> ⟶ { Expr * Expr {precedence = 1}
            { Expr / Expr {precedence = 1}
            { Expr + Expr {precedence = 0}
            { Expr - Expr {precedence = 0}

  <Int_literals> ⟶ 0| 1| ...|9 

  <letter> ⟶ a | b ...| z | A | B ... |Z

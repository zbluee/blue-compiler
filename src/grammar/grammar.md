####Grammar \( G = (N, T, P, S) \) is defined as follows:

- **Non-terminals (\( N \)):**
  - `Prog`
  - `Statement`
  - `Ident`
  - `letter`
  - `Int_literals`
  - `Expr`
  - `Term`
  - `Exprs`
  - `Scope`
  - `ConditionalBranch`

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
  - `{`
  - `}`
  - `if`
  - `elif`
  - `else`

- **Start Symbol (S):** 
    - `Prog`

- **Production Rules (\( P \)):**

  ```
  <Prog> ⟶ <Statement> | <Prog> <Statement>
  
  <Statement> ⟶  exit(<Expr>); | <let> <Ident> = <Expr> | <Scope> | if(<Expr>)<Scope><ConditionalBranch>

  <Scope> ⟶ <Statement> | <Scope> <Statement>

  <ConditionalBranch> ⟶ elif(<Expr>)<Scope><ConditionalBranch> | else<Scope> | λ

  <Ident> ⟶  <letter> | <Ident> <letter> | <Ident> <Int_literals>

  <Expr> ⟶  <Term> | <Exprs>

  <Term> ⟶  <Int_literals> | <Ident> | (<Expr>)

  <Exprs> ⟶ { Expr * Expr {precedence = 1}
            { Expr / Expr {precedence = 1}
            { Expr + Expr {precedence = 0}
            { Expr - Expr {precedence = 0}

  <Int_literals> ⟶ 0| 1| ...|9 

  <letter> ⟶ a | b ...| z | A | B ... |Z

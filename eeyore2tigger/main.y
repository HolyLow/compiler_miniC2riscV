
%token INTEGER VARIABLE LABEL FUNCTION
%token VAR END IF GOTO PARAM CALL RETURN
%token LOGICOP ARITHOP
%%
Goal
: GoalPart
;
GoalPart
: GoalPart VarDecl
| GoalPart FuncDecl
|
;
VarDecl
: VAR VarLength Variable
;
VarLength
: INTEGER
|
;
FuncDecl
: Function '[' INTEGER ']' FuncBody END Function
;
FuncBody
: FuncBody Expression
| FuncBody Declaration
|
;
RightValue
: Variable
| INTEGER
;
Expression
: Variable '=' RightValue Op2 RightValue
| Variable '=' Op1 RightValue
| Variable '=' RightValue
| Variable '[' RightValue ']' '=' RightValue
| Variable = Variable '[' RightValue ']'
| IF RightValue LogicalOp RightValue GOTO Label
| GOTO Label
| Label ':'
| PARAM RightValue
| Variable '=' CALL Function
| RETURN RightValue
;
Op2
: LOGICOP
| ARITHOP
| '-'
;
Op1
: '!'
: '-'
;
LogicalOp
: LOGICOP
;
Variable
: VARIABLE
;
Label
: LABEL
;
Function
: FUNCTION
;

%%

# Language Patch Reference for Simple Language (V202301)

## Purpose
In the initial definition of Simple Language (SPL), there are some ambiguous definitions, and some statements are incomplete. Therefore, we have decided to make some patches to the language to make it complete and easy to use.

We must clarify that, after the patch, the language will be somewhat loose, and some syntax may not align with the original definition. However, we will try our best to maintain compatibility.

## Patch 01: Assignment to Boolean Variables
In the initial definition of SPL, only numeric variables (integers and reals) can be assigned, not Boolean variables.

The patch is designed to add the assignment of Boolean variables using the following syntax:

```
<ID> := <BOOL_EXPR>
```

Here, the ID should be a Boolean variable, and the BOOL_EXPR should be an expression that returns a Boolean value.

## Patch 02: Polysemous Token for Identifiers
In the initial definition of SPL, an ID could be one element in an ID list, an arithmetic expression factor, or a Boolean expression factor. This can cause some reduce/reduce conflicts. To solve this, we apply the following patch:

```
ids: GP_ID "," ids | GP_ID

GP_ID: ID | ID_BOOL | ID_CHAR | ID_NUMB | ...

factor: ... | ID_NUMB

bool_factor: ... | ID_BOOL
```

Here, ID represents an identifier of undefined type, ID_CHAR represents an identifier of type char, ID_NUMB represents a numeric identifier, and ID_BOOL represents a Boolean identifier.

Before parsing the ID, we try to determine its type in the lexer by searching in the ID tables.

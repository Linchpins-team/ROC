package rbtree

import "fmt"

func (n workNode) String() string {
	if n.Node == n.NIL {
		return ""
	}
	s := ""
	s += fmt.Sprint(n.Value())
	if n.Color == BLACK {
		s = "*" + s + "*"
	}
	if n.Left != n.NIL {
		s = n.left().String() + " " + s
	}
	if n.Right != n.NIL {
		s += " " + n.right().String()
	}
	return "(" + s + ")"
}

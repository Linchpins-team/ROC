package rbtree

import "fmt"

func (n workNode) String() string {
	if n.Node == nil {
		return ""
	}
	s := ""
	s += fmt.Sprint(n.Value())
	if n.Color == BLACK {
		s = "*" + s + "*"
	}
	if n.Left != nil {
		s = n.left().String() + " " + s
	}
	if n.Right != nil {
		s += " " + n.right().String()
	}
	return "(" + s + ")"
}

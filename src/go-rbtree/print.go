package rbtree

import "fmt"

func (n *Node) String() string {
	if n == nil {
		return "()"
	}
	s := ""
	s += fmt.Sprint(n.Value())
	if n.Color == BLACK {
		s = "*" + s + "*"
	}
	if n.Left != nil {
		s = n.Left.String() + " " + s
	}
	if n.Right != nil {
		s += " " + n.Right.String()
	}
	return "(" + s + ")"
}

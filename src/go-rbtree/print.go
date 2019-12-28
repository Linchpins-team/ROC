package rbtree

import "fmt"

func (t *Tree) String() string {
	if t == nil {
		return "()"
	}
	s := ""
	if t.Left != nil {
		s += t.Left.String() + " "
	}
	s += fmt.Sprint(t.Value())
	if t.Right != nil {
		s += " " + t.Right.String()
	}
	return "(" + s + ")"
}

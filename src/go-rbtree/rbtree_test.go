package rbtree

import (
	"math/rand"
	"testing"
	"time"

	"github.com/stretchr/testify/assert"
)

type node uint64

func (n node) Value() uint64 {
	return uint64(n)
}

func TestRBTree(t *testing.T) {
	assert := assert.New(t)
	rand.Seed(time.Now().UTC().UnixNano())
	data := make([]node, 10)
	for i := range data {
		data[i] = node(rand.Int31n(100))
	}
	t.Log(data)

	// Test Insert
	tree := New(data[0])
	for _, v := range data {
		tree.Insert(v)
	}
	t.Log(tree.String())

	// shuffle the data list to random sort
	rand.Shuffle(len(data), func(i, j int) {
		data[i], data[j] = data[j], data[i]
	})

	// Test Search
	for _, v := range data {
		assert.Equal(v, tree.Search(v.Value()))
	}
	t.Log(tree.String())

	for _, v := range data {
		tree.Delete(v)
		t.Logf("Deleted %d, %s\n", v, tree)
	}
	t.Log(tree)
}

package rbtree

import (
	"math/rand"
	"testing"
	"time"

	"github.com/stretchr/testify/assert"
)

type IntValue uint64

func (n IntValue) Value() uint64 {
	return uint64(n)
}

func init() {
	rand.Seed(time.Now().UTC().UnixNano())
}

const (
	length = 1000
)

func generateTree(t *testing.T) ([]IntValue, Tree) {
	data := make([]IntValue, length)
	for i := range data {
		data[i] = IntValue(i)
	}
	shuffle(data)

	// Test Insert
	tree := New()
	for _, v := range data {
		tree.Insert(v)
		//t.Logf("insert %d %s\n", uint64(v), tree)
		tree.Check()
	}
	return data, tree
}

func TestRBTree(t *testing.T) {
	assert := assert.New(t)

	// Test Insert
	data, tree := generateTree(t)

	shuffle(data)

	// Test Search
	for _, v := range data {
		assert.Equal(v, tree.Search(v.Value()))
	}
	assert.Nil(tree.Search(uint64(length)))
	t.Log(tree.String())

	// Test Delete
	shuffle(data)
	tree.Delete()
	for _, v := range data {
		assert.Nil(tree.Search(v.Value()))
	}
}

// shuffle the data list to random sort
func shuffle(data []IntValue) {
	rand.Shuffle(len(data), func(i, j int) {
		data[i], data[j] = data[j], data[i]
	})
}

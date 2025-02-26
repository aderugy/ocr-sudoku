import React, {useEffect, useState} from 'react';

function isSafe(board, row, col, num) {
    for (let i = 0; i < 9; i++) {
        if (board[row][i] === num || board[i][col] === num) {
            return false;
        }
    }

    const startRow = Math.floor(row / 3) * 3;
    const startCol = Math.floor(col / 3) * 3;

    for (let i = startRow; i < startRow + 3; i++) {
        for (let j = startCol; j < startCol + 3; j++) {
            if (board[i][j] === num) {
                return false;
            }
        }
    }

    return true;
}

const CubeFace = ({ className }) => {
    const initialGrid = Array(9).fill(null).map(() => Array(9).fill(0));
    for (let i = 0; i < 10; i++) {
        const row = Math.floor(Math.random() * 9);
        const col = Math.floor(Math.random() * 9);
        const val = Math.floor(Math.random() * 9) + 1;

        if (isSafe(initialGrid, col, row, val))
            initialGrid[row][col] = val;
        else
            i--;
    }

    const [grid, setGrid] = useState(initialGrid);

    async function delay(ms) {
        return new Promise(resolve => {
            setTimeout(resolve, ms);
        });
    }

    useEffect( () => {
        async function animation(grid) {
            let copy = [...grid];

            async function solve() {
                for (let row = 0; row < 9; row++) {
                    for (let col = 0; col < 9; col++) {
                        if (copy[row][col] === 0) {
                            for (let num = 1; num <= 9; num++) {
                                await delay(50);

                                if (isSafe(copy, row, col, num)) {
                                    copy[row][col] = num;
                                    setGrid(copy);
                                    if (await solve()) {
                                        return true;
                                    }
                                    copy[row][col] = 0;
                                    setGrid(copy);
                                }
                            }
                            return false;
                        }
                    }
                }
            }

            await solve();
        }
        animation(grid);
    }, [grid]);

    const isThickBorder = (index) => (index + 1) % 3 === 0;

    return (
        <div className={`face ${className}`} style={{ display: 'grid', gridTemplateColumns: 'repeat(9, 1fr)' }}>
            {grid.map((row, rowIndex) =>
                row.map((cell, colIndex) => (
                    <div key={`${rowIndex}-${colIndex}`}
                         style={{
                             width: '100%',
                             height: '100%',
                             borderRight: isThickBorder(colIndex) ? '2px solid black' : '1px solid black',
                             borderBottom: isThickBorder(rowIndex) ? '2px solid black' : '1px solid black',
                             boxSizing: 'border-box',
                             display: 'flex',
                             alignItems: 'center',
                             justifyContent: 'center'
                         }}>
                        &nbsp;{cell === 0 ? '' : cell}&nbsp;
                    </div>
                ))
            )}
        </div>
    );
};

export default CubeFace;

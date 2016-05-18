from PIL import Image
import math

COLOR_TYPE = {
    (0, 0, 0): '0',
    (1, 0, 0): '1',
    (0, 1, 0): '2',
    (1, 1, 0): '3',
    (0, 0, 1): '4',
    (1, 0, 1): '5',
    (0, 1, 1): '6',
    (1, 1, 1): '7'
}

if __name__ == '__main__':

    pname = input('Image file name: ') or 'test.bmp'
    divideTo = eval(input('divide into (x, y) : ') or '32, 32')

    fin = open(pname, 'rb')
    p = Image.open(fin)
    
    size = list(p.size)
    print(size)
    for i in range(2):
        size[i] = -math.floor(-size[i] / divideTo[i]) * divideTo[i];
    p = p.resize(size)
    print(size)

    piece = [int(size[i] / divideTo[i]) for i in range(2)]


    fout = open(pname + '.txt', 'w')
    for j in range(divideTo[1]):
        for i in range(divideTo[0]):
            #part (i, j)
            color = []
            for x in range(i * piece[0], i * piece[0] + piece[0]):
                for y in range(j * piece[1], j * piece[1] + piece[1]):
                    pix = p.getpixel((x, y))
                    color.append(pix)
            pix = tuple(int(sum(x) / piece[0] / piece[1]) for x in zip(*color))
            for x in range(i * piece[0], i * piece[0] + piece[0]):
                for y in range(j * piece[1], j * piece[1] + piece[1]):
                    p.putpixel((x, y), pix)
            color = tuple(round(sum(x) / piece[0] / piece[1] / 255) for x in zip(*color))
            fout.write(COLOR_TYPE.get(color, '*'))
            
        fout.write('\n')
    
    fin.close()
    fout.close()
    
            
    p.show()

import math
import random
import pygame
import sys


class Emulator():
    def __init__(self, fname):
        self.width, self.height = 64, 32
        self.widthScaleFactor = 10
        self.heightScaleFactor = 10

        self.pixels = [0] * (64 * 32)
        self.screen = pygame.display.set_mode((self.width * self.widthScaleFactor, self.height * self.heightScaleFactor), pygame.NOFRAME)
        pygame.mixer.init()
        pygame.mixer.music.load("../Sound/Sound.mp3")

        self.mem = [0x0] * 0x1000
        self.stack = []
        self.indexReg = 0
        self.variables = [0] * 0xF1
        self.pc = 0x200
        self.keysPressed = dict([(0x0,0), (0x1,0), (0x2,0), (0x3,0), (0x4,0), (0x5,0), (0x6,0), (0x7,0), (0x8,0), (0x9,0), (0xA,0), (0xB,0), (0xC,0), (0xD,0), (0xE,0), (0xF,0)])

        self.delayTimer = 0
        self.soundTimer = 0
        self.fps = 60

        self.SetFontLocations()
        self.LoadFile(fname)

    def LoadProgramIntoMemory(self, program):
        for i in range(0, len(program)):
            self.mem[0x200 + i] = program[i]

    def LoadFile(self, fname):
        data = open(fname, 'rb').read()
        self.LoadProgramIntoMemory(data)

    def SetFontLocations(self):
        self.mem[0x0:0x50] = [
        0xF0, 0x90, 0x90, 0x90, 0xF0, #0
        0x20, 0x60, 0x20, 0x20, 0x70,
        0xF0, 0x10, 0xF0, 0x80, 0xF0,
        0xF0, 0x10, 0xF0, 0x10, 0xF0,
        0x90, 0x90, 0xF0, 0x10, 0x10,
        0xF0, 0x80, 0xF0, 0x10, 0xF0,
        0xF0, 0x80, 0xF0, 0x90, 0xF0,
        0xF0, 0x10, 0x20, 0x40, 0x40,
        0xF0, 0x90, 0xF0, 0x90, 0xF0, 
        0xF0, 0x90, 0xF0, 0x10, 0xF0,
        0xF0, 0x90, 0xF0, 0x90, 0x90,
        0xE0, 0x90, 0xE0, 0x90, 0xE0,
        0xF0, 0x80, 0x80, 0x80, 0xF0, 
        0xE0, 0x90, 0x90, 0x90, 0xE0,
        0xF0, 0x80, 0xF0, 0x80, 0xF0,
        0xF0, 0x80, 0xF0, 0x80, 0x80 ] #F


    def Draw(self, _opcode, x, y):
        width = 8
        height = _opcode & 0xF

        self.variables[0xF] = 0

        for row in range(0, height):
            sprite = self.mem[self.indexReg + row]

            for col in range(0, width):
                if sprite & 0x80 > 0:
                   if self.SetPixel(self.variables[x] + col, self.variables[y] + row) == 0:
                       self.variables[0xF] = 1
                   #will update the pixel on screen

                sprite <<= 1

    def Execute(self, _opcode):
    # first decode the opcode
        first = _opcode & 0xF000
        nnn = _opcode & 0x0FFF
        nn = _opcode & 0x00FF
        n = _opcode & 0x000F
        x = (_opcode & 0x0F00) >> 8
        y = (_opcode & 0x00F0) >> 4

        if _opcode == 0x00E0:
            self.pixels = [0] * (64 * 32)
            self.screen.fill((0, 0, 0))
            #clears the display

        elif first == 0x1000:
            self.pc = nnn

        elif first == 0x6000:
            self.variables[x] = nn

        elif first == 0xA000:
            self.indexReg = nnn

        elif first == 0xD000:
            #print("drawing")
            self.Draw(_opcode, x, y)
            #draw the pixels

        elif first == 0x2000:
            self.stack.append(self.pc)
            self.pc = nnn
        
        elif _opcode == 0x00EE:
            self.pc = self.stack[-1]
            del self.stack[-1]

        elif first == 0x3000:
            if self.variables[x] == nn:
                self.pc += 2

        elif first == 0x4000:
            if self.variables[x] != nn:
                self.pc += 2
        
        elif first == 0x5000:
            if self.variables[x] == self.variables[y]:
                self.pc += 2
        
        elif first == 0x9000:
            if self.variables[x] != self.variables[y]:
                self.pc += 2

        elif first == 0x7000:
            self.variables[x] += (_opcode & 0xFF)
            self.variables[x] &= 0xFF

        elif first == 0x8000:
            if n == 0x0:
                self.variables[x] = self.variables[y]
            
            elif n == 0x1:
                self.variables[x] |= self.variables[y]

            elif n == 0x2:
                self.variables[x] &= self.variables[y]

            elif n == 0x3:
                self.variables[x] ^= self.variables[y]

            elif n == 0x0004:
                self.variables[x] += self.variables[y]
                if self.variables[x] > 0xFF:
                    self.variables[0xF] = 1
                else:
                    self.variables[0xF] = 0

                self.variables[x] = self.variables[x] & 0xFF

            elif n == 0x0005:
                if self.variables[x] > self.variables[y]:
                    self.variables[0xF] = 1
                elif self.variables[x] < self.variables[y]:
                    self.variables[0xF] = 0

                self.variables[x] = self.variables[x] - self.variables[y]
                self.variables[x] = self.variables[x] & 0xFF

            elif n == 0x0007:
                if self.variables[y] > self.variables[x]:
                    self.variables[0xF] = 1
                elif self.variables[y] < self.variables[x]:
                    self.variables[0xF] = 0

                self.variables[x] = self.variables[y] - self.variables[x]
                self.variables[x] = self.variables[x] & 0xFF

            elif n == 0x0006:
                #old implementation
                #self.variables[x] = self.variables[y]
                ####

                self.variables[0xF] = (self.variables[x] & 0x1)

                self.variables[x] >>= 1

            elif n == 0xE:
                #old implementation
                #self.variables[x] = self.variables[y]
                ####

                self.variables[0xF] = ((self.variables[x] & 0x80) >> 7)
                self.variables[x] <<= 1

        elif first == 0xA000:
            self.indexReg = nnn

        elif first == 0xB000:
            #old implementation
            #self.pc = nnn + self.variables[0x0]
            ####

            self.pc = nnn + self.variables[x]

        elif first == 0xC000:
            num = random.randrange(0, 256)
            self.variables[x] = num & nn

        elif first == 0xE000:
            if nn == 0x009E:
                if self.keysPressed[self.variables[x]] == 1:
                    self.pc += 2
            elif nn == 0x00A1:
                if self.keysPressed[self.variables[x]] == 0:
                    self.pc += 2

        elif first == 0xF000:
            if nn == 0x07:
                self.variables[x] = self.delayTimer
            
            elif nn == 0x15:
                self.delayTimer = self.variables[x]

            elif nn == 0x18:
                self.soundTimer = self.variables[x]

            elif nn == 0x1E:
                self.indexReg += self.variables[x]
                #check this one

            elif nn == 0x0A:
                if all(x == 0 for x in self.keysPressed.values()):
                    self.pc -= 2
                else:
                    self.variables[x] = list(self.keysPressed.keys())[list(self.keysPressed.values()).index(1)]

            elif nn == 0x29:
                self.indexReg = self.variables[x] + self.variables[x] * 4

            elif nn == 0x33:
                num = self.variables[x]
                ones = int(num % 10)
                tenths = int(((num - ones) / 10) % 10)
                hundreths = int((num - ones - tenths * 10) / 100)

                self.mem[self.indexReg] = hundreths
                self.mem[self.indexReg + 1] = tenths
                self.mem[self.indexReg + 2] = ones

            elif nn == 0x55:
                for i in range(0, x + 1):
                    self.mem[self.indexReg + i] = self.variables[i]

            elif nn == 0x65:
                for i in range(0, x + 1):
                    self.variables[i] = self.mem[self.indexReg + i]


        

    def SetPixel(self, x, y):
        if x > self.width - 1:
            x = 0
        elif x < 0:
            x = self.width - 1

        if y > self.height - 1:
            y = 0
        elif y < 0:
            y = self.height - 1

        pixelLoc = x + (y * self.width)
        self.pixels[pixelLoc] ^= 1

        return self.pixels[pixelLoc]

    def Render(self):
        self.screen.fill((0, 0, 0))

        for i in range(0, self.width * self.height):
            x = i % self.width
            y = math.floor(i / self.width)

            if self.pixels[i] == 1:
                pygame.draw.rect(self.screen, (255, 255, 255), pygame.Rect(x * self.widthScaleFactor, y * self.heightScaleFactor, 1 * self.widthScaleFactor, 1 * self.heightScaleFactor))

        #draw a debugging grid
        #self.RenderGrid()

    def Cycle(self):
        speed = 1
        for i in range(0, speed):
            opcode = ((self.mem[self.pc]<<8) | self.mem[self.pc + 1])
            self.pc += 2
            self.Execute(opcode)

        if self.delayTimer > 0:
            self.delayTimer -= 1

        if self.soundTimer > 0:
            self.soundTimer -= 1

    def RenderGrid(self):
        for x in range(0, self.width):
            pygame.draw.line(self.screen, (3, 7, 252), (x * self.widthScaleFactor, 0), (x * self.widthScaleFactor, self.height * self.heightScaleFactor))

        for y in range(0, self.height):
            pygame.draw.line(self.screen, (3, 7, 252), (0, y * self.heightScaleFactor), (self.width * self.widthScaleFactor, y * self.heightScaleFactor))
        

    def Main(self):
        running = True
        interval = self.fps / 1000
        clock = pygame.time.Clock()
        timer = 0

        while running:
            timer += pygame.time.get_ticks()

            if self.soundTimer != 0:
                pygame.mixer.music.play()
            else:
                pygame.mixer.music.stop()

            if timer > interval:
                timer = 0

                for event in pygame.event.get():
                    if event.type == pygame.QUIT:
                        running = False
                    if event.type == pygame.KEYDOWN:
                        if event.key == pygame.K_1:
                            self.keysPressed[0x1] = 1
                        if event.key == pygame.K_2:
                            self.keysPressed[0x2] = 1
                        if event.key == pygame.K_3:
                            self.keysPressed[0x3] = 1
                        if event.key == pygame.K_4:
                            self.keysPressed[0xC] = 1
                        if event.key == pygame.K_q:
                            self.keysPressed[0x4] = 1
                        if event.key == pygame.K_w:
                            self.keysPressed[0x5] = 1
                        if event.key == pygame.K_e:
                            self.keysPressed[0x6] = 1
                        if event.key == pygame.K_r:
                            self.keysPressed[0xD] = 1
                        if event.key == pygame.K_a:
                            self.keysPressed[0x7] = 1
                        if event.key == pygame.K_s:
                            self.keysPressed[0x8] = 1
                        if event.key == pygame.K_d:
                            self.keysPressed[0x9] = 1
                        if event.key == pygame.K_f:
                            self.keysPressed[0xE] = 1
                        if event.key == pygame.K_z:
                            self.keysPressed[0xA] = 1
                        if event.key == pygame.K_x:
                            self.keysPressed[0x0] = 1
                        if event.key == pygame.K_c:
                            self.keysPressed[0xB] = 1
                        if event.key == pygame.K_v:
                            self.keysPressed[0xF] = 1
                        if event.key == pygame.K_ESCAPE:
                            running = False

                    elif event.type == pygame.KEYUP:
                        if event.key == pygame.K_1:
                            self.keysPressed[0x1] = 0
                        if event.key == pygame.K_2:
                            self.keysPressed[0x2] = 0
                        if event.key == pygame.K_3:
                            self.keysPressed[0x3] = 0
                        if event.key == pygame.K_4:
                            self.keysPressed[0xC] = 0
                        if event.key == pygame.K_q:
                            self.keysPressed[0x4] = 0
                        if event.key == pygame.K_w:
                            self.keysPressed[0x5] = 0
                        if event.key == pygame.K_e:
                            self.keysPressed[0x6] = 0
                        if event.key == pygame.K_r:
                            self.keysPressed[0xD] = 0
                        if event.key == pygame.K_a:
                            self.keysPressed[0x7] = 0
                        if event.key == pygame.K_s:
                            self.keysPressed[0x8] = 0
                        if event.key == pygame.K_d:
                            self.keysPressed[0x9] = 0
                        if event.key == pygame.K_f:
                            self.keysPressed[0xE] = 0
                        if event.key == pygame.K_z:
                            self.keysPressed[0xA] = 0
                        if event.key == pygame.K_x:
                            self.keysPressed[0x0] = 0
                        if event.key == pygame.K_c:
                            self.keysPressed[0xB] = 0
                        if event.key == pygame.K_v:
                            self.keysPressed[0xF] = 0

                self.Cycle()
                self.Render()
                
            pygame.display.flip()


#create instance of emulator
emulator = Emulator(sys.argv[1]) #use sys args when done
emulator.Main()
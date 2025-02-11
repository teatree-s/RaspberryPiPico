class Jellyfish:
    def __init__(self, images=[], images_order=[], position=0):
        self.images = images
        self.data = images_order
        self.image_index = 0
        self._x = 0
        self._y = 0
        self.order_x_index = 0
        self.order_y_index = 0
        self.x_values = self.oscillate(0, 110, 2, position)
        self.y_values = self.oscillate(0, 46, 2, position)

    def oscillate(self, start, stop, step, position):
        seq = list(range(start, stop + 1, step)) + list(
            range(stop - step, start - 1, -step)
        )
        if position:
            half_index = len(seq) // position
            seq = seq[half_index:] + seq[:half_index]
            self.order_index = half_index
        return seq

    def get_position(self):
        return self._x, self._y

    def get_image(self):
        return self.image

    def get_next_image(self):
        if not self.images or not self.data:
            return None
        image = self.images[self.data["images"][self.image_index]]
        self.image_index = (self.image_index + 1) % len(self.data["images"])
        return image

    def update(self):
        self._x = self.x_values[self.order_x_index]
        self.order_x_index = (self.order_x_index + 1) % len(self.x_values)
        self._y = self.y_values[self.order_y_index]
        self.order_y_index = (self.order_y_index + 1) % len(self.y_values)
        self.image = self.get_next_image()

    @property
    def x(self):
        return self._x

    @property
    def y(self):
        return self._y

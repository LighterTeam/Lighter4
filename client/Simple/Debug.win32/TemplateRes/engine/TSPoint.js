TSPoint = function (x, y) {
    this.m_x = parseInt(x);
    this.m_y = parseInt(y);
    this.add = function(p) {
        this.m_x += p.m_x;
        this.m_y += p.m_y;
    };
    this.sub = function(p) {
        this.m_x -= p.m_x;
        this.m_y -= p.m_y;
    }
    this.set = function(p) {
        this.m_x = p.m_x;
        this.m_y = p.m_y;
    };
    this.set = function(x,y) {
        this.m_x = parseInt(x);
        this.m_y = parseInt(y);
    }
    this.get = function() {
        var newobj = new TSPoint(this.m_x, this.m_y);
        return newobj;
    }
    this.getX = function() {
        return this.m_x;
    }
    this.getY = function() {
        return this.m_y;
    }
    this.equal = function(p) {
        if (this.m_x == p.m_x && this.m_y == p.m_y){
            return true;
        }
        return false;
    }
};





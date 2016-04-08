
(define (cycle i max)
  (if (> i max)
      (display i)
      (cycle (+ i 1) max)))

(cycle 0 665)

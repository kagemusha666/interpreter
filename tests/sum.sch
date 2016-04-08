
(define (run n)
  (define (loop i sum) 
    (if (< i 0)
      (display sum)
      (loop (- i 1) (+ 1 sum))))
  (loop n 0))

(run 665)

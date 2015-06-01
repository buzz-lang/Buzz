(require 'newcomment)

;; Command to comment/uncomment text
(defun buzz-comment-dwim (arg)
  "Comment or uncomment current line or region in a smart way.
For detail, see `comment-dwim'."
  (interactive "*P")
  (let ((comment-start "#") (comment-end ""))
    (comment-dwim arg)))

;; Buzz syntax table
(defvar buzz-syntax-table nil "Syntax table for `buzz-mode'.")
(setq buzz-syntax-table
      (let ((synTable (make-syntax-table)))
        (modify-syntax-entry ?# "< b" synTable)
        (modify-syntax-entry ?\n "> b" synTable)
        synTable))

;;
;; Define token classes and regular expressions
;;
;; Keywords
(setq buzz-keywords '("var" "nil" "if" "else" "function" "return" "for" "while" "and" "or" "not" "swarm" "stigmergy" "neighbors" "self" "id" "math"))
(setq buzz-keywords-regexp (regexp-opt buzz-keywords 'words))
(setq buzz-keywords nil)

;;
;; Create font lock list
;;
(setq buzz-font-lock-list
      `(
        (,buzz-keywords-regexp . font-lock-keyword-face)
        ))

;;
;; Define the mode
;;
(define-derived-mode buzz-mode prog-mode
  "Buzz mode"
  "Major mode for editing Buzz scripts"
  :syntax-table buzz-syntax-table
  ;; Set font lock
  (setq font-lock-defaults '((buzz-font-lock-list)))
  ;; Modify the keymap
  (define-key buzz-mode-map [remap comment-dwim] 'buzz-comment-dwim)
  ;; Clear memory
  (setq buzz-keywords-regexp nil)
  )

(provide 'buzz-mode)

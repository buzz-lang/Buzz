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
(defconst buzz-identifier-regexp "[[:alnum:]_]+"
  "Regexp matching a Buzz identifier")
;; Keywords
(setq buzz-keywords '("var" "nil" "if" "else" "function" "return" "for" "while" "and" "or" "not"))
(setq buzz-keywords-regexp (regexp-opt buzz-keywords 'words))
(setq buzz-keywords nil)
;; Builtins
(setq buzz-builtins '("swarm" "stigmergy" "neighbors" "self" "id" "math"))
(setq buzz-builtins-regexp (regexp-opt buzz-builtins 'words))
(setq buzz-builtins nil)
;; Functions
(setq buzz-functions-regexp (concat "^\\s-*function\\s-+\\(" buzz-identifier-regexp "\\)"))

;;
;; Create font lock list
;;
(setq buzz-font-lock-list
      `(
        (,buzz-builtins-regexp  . font-lock-builtin-face)
        (,buzz-functions-regexp 1 font-lock-function-name-face)
        (,buzz-keywords-regexp  . font-lock-keyword-face)
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
  (setq buzz-functions-regexp nil)
)

(provide 'buzz-mode)

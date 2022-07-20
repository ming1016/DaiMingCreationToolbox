;; -*- lexical-binding: t -*-

;; ------------ orgmode ----------
(setq org-image-actual-width '(500))  ;; 设置图片默认宽度
(setq org-src-fontify-natively t)

;; ------------ 插件管理 -----------

;; 懒猫的 awesome-tab 多 tab 切换文件
(add-to-list 'load-path (expand-file-name "~/.emacs.d/awesome-tab/"))

(require 'awesome-tab)

(awesome-tab-mode t)

(defun awesome-tab-buffer-groups ()
"`awesome-tab-buffer-groups' control buffers' group rules.
Group awesome-tab with mode if buffer is derived from `eshell-mode' `emacs-lisp-mode' `dired-mode' `org-mode' `magit-mode'.
All buffer name start with * will group to \"Emacs\".
Other buffer group by `awesome-tab-get-group-name' with project name."
(list
(cond
    ((or (string-equal "*" (substring (buffer-name) 0 1))
  (memq major-mode '(magit-process-mode
          magit-status-mode
          magit-diff-mode
          magit-log-mode
          magit-file-mode
          magit-blob-mode
          magit-blame-mode)))
    "Emacs")
    ((derived-mode-p 'eshell-mode)
    "EShell")
    ((derived-mode-p 'dired-mode)
    "Dired")
    ((memq major-mode '(org-mode org-agenda-mode diary-mode))
    "OrgMode")
    ((derived-mode-p 'eaf-mode)
    "EAF")
    (t
    (awesome-tab-get-group-name (current-buffer))))))

;; 增强文件内搜索和跳转函数定义
(package-install 'consult)
(global-set-key (kbd "C-s") 'consult-line)

;; modeline 完善
(package-install 'embark)
(global-set-key (kbd "C-;") 'embark-act)
(setq prefix-help-command 'embark-prefix-help-command)

;; modeline 里的内容加上注释
(package-install 'marginalia)
(marginalia-mode t)

;; modeline 是垂直
(package-install 'vertico)
(vertico-mode t)

;; 通过空格模糊搜索
(package-install 'orderless)
(setq completion-styles '(orderless))

;; 在 modeline 上显示按键和执行命令
(package-install 'keycast)
(keycast-mode t)

;; 快捷键提示
(package-install 'which-key)
(which-key-mode t)


;; ---- 自定义快捷键 ------
;; 查找文档快捷键
(global-set-key (kbd "C-h C-f") 'find-function)
(global-set-key (kbd "C-h C-v") 'find-variable)
(global-set-key (kbd "C-h C-k") 'find-function-on-key)

;; 查看最近文件
(require 'recentf)
(recentf-mode 1)
(setq recentf-max-menu-items 10)
(global-set-key (kbd "C-x C-r") 'recentf-open-files)

;; f12 打开日记
(defun open-diary-file()
  (interactive)
  (find-file "~/Documents/Gitee/OrgModeFile/org/today.org"))
(global-set-key (kbd "<f12>") 'open-diary-file)

;; F1 打开配置文件
(defun open-init-file()
  (interactive)
  (find-file "~/.emacs.d/init.el"))
(global-set-key (kbd "<f2>") 'open-init-file) ;; 使用 F2 键来打开初始配置文件

;; ------- 功能完善 -------
(global-hl-line-mode 0) ;; 当前行高亮
;; 自动换行
(global-visual-line-mode 1)
(setq-default truncate-lines t)
(add-hook 'org-mode-hook
    (lambda()
      (setq truncate-lines nil)))

(desktop-save-mode 0) ;; 保存当前窗口，下次启动恢复
(setq make-backup-files nil) ;; 删除 ~ 这样的用来备份的文件
(global-company-mode 1) ;; 补全
(setq company-idle-delay 0) ;; 补全加速
(electric-pair-mode 1) ;; 括号匹配
(show-paren-mode t) ;; 括号匹配错误提示
(delete-selection-mode 1) ;; 选中区域输入新内容时，自动删除选中区域

;; -------- GUI 完善 ------
(setq mac-option-modifier 'meta
      mac-command-modifier 'super
      mac-right-option-modifier 'control
      )

;;让鼠标滚动更好用
(setq mouse-wheel-scroll-amount '(1 ((shift) . 1) ((control) . nil)))
(setq mouse-wheel-progressive-speed nil)

(set-face-attribute 'default nil :height 120) ;; 字体设置为12pt
;;(add-to-list 'default-frame-alist
;;             '(font . "LXGW WenKai-14"))
(tool-bar-mode -1) ;; 工具栏
(scroll-bar-mode -1) ;; 滚动栏
(global-linum-mode 0) ;; 行号
(setq-default cursor-type 'bar) ;; 更改
(icomplete-mode 1)

(custom-set-variables
 ;; custom-set-variables was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 '(package-selected-packages
   '(neotree which-key consult embark marginalia orderless vertico keycast org company)))
(custom-set-faces
 ;; custom-set-faces was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 )






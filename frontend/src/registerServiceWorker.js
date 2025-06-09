import { Workbox } from 'workbox-window'

export function registerServiceWorker() {
  if ('serviceWorker' in navigator) {
    const wb = new Workbox('/sw.js')

    wb.addEventListener('waiting', () => {
      // Immediately skip waiting and activate new service worker
      wb.messageSW({ type: 'SKIP_WAITING' })
    })

    wb.addEventListener('controlling', () => {
      // New service worker has taken control, reload the page
      window.location.reload()
    })

    wb.register()
  }
}
